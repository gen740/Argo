module;

#include <cassert>

export module Argo;

export import :Exceptions;
export import :Validation;
export import :Initializer;

import :TypeTraits;
import :MetaAssigner;
import :MetaChecker;
import :MetaLookup;
import :Arg;
import :NArgs;
import :HelpGenerator;
import :std_module;

namespace Argo {

struct Unspecified {};

template <auto Value>
struct IdentityHolder {
  static constexpr auto value = Value;
};

/*!
 * convert const char[] to std::array<char, N>
 */
export template <std::size_t N>
consteval auto arg(const char (&a)[N]) {
  std::array<char, N - 1> arr{};
  for (std::size_t i = 0; i < N - 1; ++i) {
    arr[i] = a[i];
  }
  return arr;
}

/*!
 * Helper function to create nargs
 */
export {
  consteval auto nargs(char narg) {
    return NArgs(narg);
  }

  consteval auto nargs(int narg) {
    return NArgs(narg);
  }
}

template <char Name>
struct ShortName {
  static constexpr char value = Name;
};

// consteval auto short(char a)  {
//   return ShortName<a>();
// }

export template <int ID = 0, class Args = std::tuple<>>
class Parser {
 private:
  bool parsed_ = false;
  std::string_view programName_;

 public:
  constexpr explicit Parser() = default;
  constexpr explicit Parser(std::string_view programName) : programName_(programName){};

  using Arguments = Args;
  Args value;

  /*!
   * Type: type of argument
   * Name: name of argument
   * arg1: ShortName or NArgs or unspecified
   * arg2: NArgs or unspecified
   */
  template <class Type, auto Name, auto arg1 = Unspecified(), auto arg2 = Unspecified(), class... T>
  auto addArg(T... args) {
    static constexpr char ShortName = std::conditional_t<        //
        std::is_same_v<std::remove_cv_t<decltype(arg1)>, char>,  //
        IdentityHolder<arg1>,                                    //
        IdentityHolder<NULLCHAR>                                 //
        >::value;

    static constexpr auto nargs = std::conditional_t<                       //
        std::derived_from<std::remove_cvref_t<decltype(arg1)>, NArgs>,      //
        IdentityHolder<arg1>,                                               //
        std::conditional_t<                                                 //
            std::derived_from<std::remove_cvref_t<decltype(arg2)>, NArgs>,  //
            IdentityHolder<arg2>,                                           //
            IdentityHolder<NArgs('?')>>>::value;

    static_assert(
        (ShortName == NULLCHAR) || (SearchIndexFromShortName<Arguments, ShortName>::value == -1),
        "Duplicated short name");
    static_assert(                                        //
        Argo::SearchIndex<Arguments, Name>::value == -1,  //
        "Duplicated name");

    ArgInitializer<Type, Name, ShortName, nargs, ID>::init(std::forward<T>(args)...);
    return Parser<ID, decltype(std::tuple_cat(                                               //
                          std::declval<Arguments>(),                                         //
                          std::declval<std::tuple<Arg<Type, Name, ShortName, nargs, ID>>>()  //
                          ))>();
  }

  template <auto Name, char ShortName = NULLCHAR, class... T>
  auto addFlag(T... args) {
    static_assert(
        (ShortName == NULLCHAR) || (SearchIndexFromShortName<Arguments, ShortName>::value == -1),
        "Duplicated short name");
    static_assert(                                        //
        Argo::SearchIndex<Arguments, Name>::value == -1,  //
        "Duplicated name");

    FlagArgInitializer<Name, ShortName, ID>::init(std::forward<T>(args)...);
    return Parser<ID, decltype(std::tuple_cat(                                      //
                          std::declval<Arguments>(),                                //
                          std::declval<std::tuple<FlagArg<Name, ShortName, ID>>>()  //
                          ))>();
  }

  template <auto Name>
  auto getArg() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    using P =
        std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(this->value))>;
    return P::value;
  }

  template <auto Name>
  auto getArgOr(std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(
                    std::declval<Arguments>))>::type val) {
    return this->getArg<Name>.value_or(val);
  }

 private:
  /*!
   * Setting the argument value
   */
  auto setArg(std::string_view key, std::string_view val) {
    Assigner::assign<Arguments>(key, val);
  }

  auto setArg(std::string_view key, const std::vector<std::string_view>& val) {
    VariadicAssigner::assign<Arguments>(key, val);
  }

  auto setDefault(std::string_view key) {
    DefaultAssigner::assign<Arguments>(key);
  }

 public:
  /*!
   * Parse State
   */
  enum class ArgumentParseState {
    ExpectValue,
    ExpectFlag,
    ExpectNValuesOrFlag,
    ExpectOneValuesOrFlag,
    ExpectNValues,
    ExpectOneValue,
    // Expect,
  };

  std::vector<ArgInfo> getArgInfo() {
    return HelpGenerator::generate<Arguments>();
  }

  std::string formatHelp() {
    std::string help;
    auto helpInfo = this->getArgInfo();
    std::size_t maxFlagLength = 0;
    for (const auto& i : helpInfo) {
      if (maxFlagLength < i.name.size()) {
        maxFlagLength = i.name.size();
      }
    }
    help.append("Options:");
    for (const auto& i : helpInfo) {
      help.push_back('\n');
      help.append(std::format(                                                   //
          "  {} --{} {} {}",                                                     //
          (i.shortName == NULLCHAR) ? "   " : std::format("-{},", i.shortName),  //
          i.name,                                                                //
          std::string(maxFlagLength - i.name.size(), ' '),                       //
          i.description                                                          //
          ));
    }
    return help;
  }

  /*!
   * Parse function
   */
  auto parse([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) -> void {
    auto parseState = ArgumentParseState::ExpectFlag;
    std::string_view argBuffer;
    int expectN = 0;
    [[maybe_unused]] bool atLeastOne = false;
    auto variadicValueBuffer = std::vector<std::string_view>();
    for (int i = 1; i < argc; i++) {
      auto argValue = std::string_view(argv[i]);
      switch (parseState) {
        case ArgumentParseState::ExpectNValues:
          variadicValueBuffer.push_back(argValue);
          expectN--;
          if (expectN == 0) {
            this->setArg(argBuffer, variadicValueBuffer);
            variadicValueBuffer.clear();
            parseState = ArgumentParseState::ExpectFlag;
            break;
          }
          break;
        case ArgumentParseState::ExpectOneValue:
          this->setArg(argBuffer, argBuffer);
          parseState = ArgumentParseState::ExpectFlag;
          break;
        case ArgumentParseState::ExpectOneValuesOrFlag:
          if (argValue.starts_with("-")) {
            parseState = ArgumentParseState::ExpectFlag;
            this->setDefault(argBuffer);
            goto ParseFlag;
          }
          this->setArg(argBuffer, argValue);
          parseState = ArgumentParseState::ExpectFlag;
          break;
        case ArgumentParseState::ExpectNValuesOrFlag:
          if (argValue.starts_with("-")) {
            if (variadicValueBuffer.empty()) {
              if (atLeastOne) {
                throw InvalidArgument(
                    std::format("Argument {} need at least one value", argBuffer));
              }
              parseState = ArgumentParseState::ExpectFlag;
              this->setDefault(argBuffer);
              goto ParseFlag;
            }
            parseState = ArgumentParseState::ExpectFlag;
            this->setArg(argBuffer, variadicValueBuffer);
            goto ParseFlag;
          }
          variadicValueBuffer.push_back(argValue);
          if (i == (argc - 1)) {
            this->setArg(argBuffer, variadicValueBuffer);
            break;
          }
          break;
        ParseFlag:
        case ArgumentParseState::ExpectFlag:
          if (argValue.starts_with("--")) {
            if (argValue.contains("=")) {  // equal assign
              auto find_pos = argValue.find('=');
              this->setArg(argValue.substr(2, find_pos - 2), argValue.substr(find_pos + 1));
              break;
            }
            argBuffer = argValue.substr(2);
            auto check = Checker::check<Arguments>(argBuffer);

            if (check.isFlag) {
              this->setArg(argBuffer, "true");
              break;
            }
            if (check.nargs.nargs_char == '?') {
              parseState = ArgumentParseState::ExpectOneValuesOrFlag;
              if (i == argc - 1) {
                this->setDefault(argBuffer);
              }
              break;
            }
            if (check.nargs.nargs_char == '*') {
              atLeastOne = false;
              parseState = ArgumentParseState::ExpectNValuesOrFlag;
              if (i == argc - 1) {
                this->setDefault(argBuffer);
              }
              break;
            }
            if (check.nargs.nargs_char == '+') {
              atLeastOne = true;
              parseState = ArgumentParseState::ExpectNValuesOrFlag;
              if (i == argc - 1) {
                throw InvalidArgument(
                    std::format("Argument {} need at least one value", argBuffer));
              }
              break;
            }
            if (check.nargs.nargs == 1) {
              parseState = ArgumentParseState::ExpectOneValue;
            } else if (check.nargs.nargs > 1) {
              expectN = check.nargs.nargs;
              parseState = ArgumentParseState::ExpectNValues;
              break;
            }
            parseState = ArgumentParseState::ExpectValue;
            break;
          } else if (argValue.starts_with('-')) {
            auto shortArg = argValue.substr(1);
            argBuffer = "";
            for (char i : shortArg) {
              auto optionName = GetNameFromShortName<Arguments>::eval(i);
              auto check = Checker::check<Arguments>(optionName);

              if (check.isFlag) {
                this->setArg(optionName, "true");
              } else {
                if (!argBuffer.empty()) {
                  throw Argo::InvalidArgument("Combining two more optional argument");
                }
                argBuffer = std::string(optionName);  // copy
              }
            }
            if (argBuffer.empty()) {
              break;
            }
            parseState = ArgumentParseState::ExpectValue;
            break;
          } else {
            throw Argo::InvalidArgument("Expect flag");
          }
          assert(false);  // connot reach
        case ArgumentParseState::ExpectValue:
          if (argValue.starts_with('-')) {
            throw Argo::InvalidArgument("Expect value");
          }
          this->setArg(argBuffer, argValue);
          argBuffer = "";
          parseState = ArgumentParseState::ExpectFlag;
          break;
      }
    }
    this->parsed_ = true;
  }
};

}  // namespace Argo
