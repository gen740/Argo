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
    auto parse_state = ArgumentParseState::ExpectFlag;
    std::string_view arg_buffer;
    std::string tmp_arg;
    int expect_N = 0;
    bool at_least_one = false;
    auto variadic_value_buffer = std::vector<std::string_view>();

    auto process_nargs = [this, &parse_state, &argc, &arg_buffer, &at_least_one, &expect_N](
                             int i, NArgs nargs) {
      if (nargs.nargs_char == '?') {
        parse_state = ArgumentParseState::ExpectOneValuesOrFlag;
        if (i == argc - 1) {
          this->setDefault(arg_buffer);
        }
        return true;
      }
      if (nargs.nargs_char == '*') {
        at_least_one = false;
        parse_state = ArgumentParseState::ExpectNValuesOrFlag;
        if (i == argc - 1) {
          this->setDefault(arg_buffer);
        }
        return true;
      }
      if (nargs.nargs_char == '+') {
        at_least_one = true;
        parse_state = ArgumentParseState::ExpectNValuesOrFlag;
        if (i == argc - 1) {
          throw InvalidArgument(std::format("Argument {} need at least one value", arg_buffer));
        }
        return true;
      }
      if (nargs.nargs == 1) {
        parse_state = ArgumentParseState::ExpectOneValue;
      } else if (nargs.nargs > 1) {
        expect_N = nargs.nargs;
        parse_state = ArgumentParseState::ExpectNValues;
        return true;
      }
      return false;
    };

    for (int i = 1; i < argc; i++) {
      auto arg_value = std::string_view(argv[i]);
      switch (parse_state) {
        case ArgumentParseState::ExpectNValues:
          variadic_value_buffer.push_back(arg_value);
          expect_N--;
          if (expect_N == 0) {
            this->setArg(arg_buffer, variadic_value_buffer);
            variadic_value_buffer.clear();
            parse_state = ArgumentParseState::ExpectFlag;
            break;
          }
          break;
        case ArgumentParseState::ExpectOneValue:
          this->setArg(arg_buffer, arg_buffer);
          parse_state = ArgumentParseState::ExpectFlag;
          break;
        case ArgumentParseState::ExpectOneValuesOrFlag:
          if (arg_value.starts_with("-")) {
            parse_state = ArgumentParseState::ExpectFlag;
            this->setDefault(arg_buffer);
            goto ParseFlag;
          }
          this->setArg(arg_buffer, arg_value);
          parse_state = ArgumentParseState::ExpectFlag;
          break;
        case ArgumentParseState::ExpectNValuesOrFlag:
          if (arg_value.starts_with("-")) {
            if (variadic_value_buffer.empty()) {
              if (at_least_one) {
                throw InvalidArgument(
                    std::format("Argument {} need at least one value", arg_buffer));
              }
              parse_state = ArgumentParseState::ExpectFlag;
              this->setDefault(arg_buffer);
              goto ParseFlag;
            }
            parse_state = ArgumentParseState::ExpectFlag;
            this->setArg(arg_buffer, variadic_value_buffer);
            goto ParseFlag;
          }
          variadic_value_buffer.push_back(arg_value);
          if (i == (argc - 1)) {
            this->setArg(arg_buffer, variadic_value_buffer);
            break;
          }
          break;
        ParseFlag:
        case ArgumentParseState::ExpectFlag:
          if (arg_value.starts_with("--")) {
            if (arg_value.contains("=")) {  // equal assign
              auto find_pos = arg_value.find('=');
              this->setArg(arg_value.substr(2, find_pos - 2), arg_value.substr(find_pos + 1));
              break;
            }
            arg_buffer = arg_value.substr(2);
            auto check = Checker::check<Arguments>(arg_buffer);

            if (check.isFlag) {
              this->setArg(arg_buffer, "true");
              break;
            }

            if (process_nargs(i, check.nargs)) {
              break;
            }
            parse_state = ArgumentParseState::ExpectValue;
            break;
          } else if (arg_value.starts_with('-')) {
            auto short_arg = arg_value.substr(1);
            arg_buffer = "";
            for (char i : short_arg) {
              auto option_name = GetNameFromShortName<Arguments>::eval(i);
              auto check = Checker::check<Arguments>(option_name);

              if (check.isFlag) {
                this->setArg(option_name, "true");
              } else {
                if (!arg_buffer.empty()) {
                  throw Argo::InvalidArgument("Combining two more optional argument");
                }
                // copy and extend lifetime
                tmp_arg = std::string(option_name.begin(), option_name.end());
                arg_buffer = tmp_arg;
              }
            }
            if (arg_buffer.empty()) {
              break;
            }
            parse_state = ArgumentParseState::ExpectValue;
            break;
          } else {
            throw Argo::InvalidArgument("Expect flag");
          }
          assert(false);  // connot reach
        case ArgumentParseState::ExpectValue:
          if (arg_value.starts_with('-')) {
            throw Argo::InvalidArgument("Expect value");
          }
          this->setArg(arg_buffer, arg_value);
          arg_buffer = "";
          parse_state = ArgumentParseState::ExpectFlag;
          break;
      }
    }
    this->parsed_ = true;
  }
};

}  // namespace Argo
