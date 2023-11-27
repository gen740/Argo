module;

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

template <int N>
struct A {};

template <bool B>
using EnableIf = std::enable_if_t<B, int>;
/*!
 * Helper function to create nargs
 */
export {
  consteval auto nargs(char narg) -> NArgs {
    return NArgs(narg);
  }

  consteval auto nargs(int narg) -> NArgs {
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
    static_assert(                     //
        (nargs.nargs > 0               //
         || nargs.nargs_char == '?'    //
         || nargs.nargs_char == '+'    //
         || nargs.nargs_char == '*'),  //
        "nargs must be '?', '+', '*' or int");

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
    using V =
        std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(this->value))>;
    return V::value;
  }

  template <auto Name>
  auto getArgOr(std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(
                    std::declval<Arguments>))>::type val) {
    return this->getArg<Name>.value_or(val);
  }

  template <auto Name>
  auto getArgOrDefault() {
    using V =
        std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(this->value))>;
    return V::value || V::defaultValue;
  }

  template <auto Name>
  auto getArgOrDefaultOr(std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(
                             std::declval<Arguments>))>::type val) {
    return this->getArgOrDefault().value_or(val);
  }

 private:
  auto setArg(std::string_view key, const std::vector<std::string_view>& val) const {
    Assigner::assign<Arguments>(key, val);
  }

  auto setArg(const std::vector<char>& key, const std::vector<std::string_view>& val) const {
    Assigner::assign<Arguments>(key, val);
  }

 public:
  auto parse(int argc, char* argv[]) -> void {
    std::string_view key{};
    std::vector<char> short_keys{};
    short_keys.reserve(10);
    std::vector<std::string_view> values{};
    values.reserve(10);

    for (int i = 1; i < argc; i++) {
      std::string_view arg = argv[i];
      if (arg.starts_with('-')) {
        if (arg.starts_with("--")) {  // start with --
          if (!key.empty()) {
            this->setArg(key, values);
            key = "";
            values.clear();
          } else if (!short_keys.empty()) {
            this->setArg(short_keys, values);
            short_keys.clear();
            values.clear();
          }
          if (arg.contains('=')) {
            auto equal_pos = arg.find('=');
            this->setArg(arg.substr(2, equal_pos - 2), {arg.substr(equal_pos + 1)});
            continue;
          }
          key = arg.substr(2);
          if (i == (argc - 1)) {
            this->setArg(key, {});
          }
          continue;
        }
        if (not key.empty()) {
          this->setArg(key, values);
          key = "";
          values.clear();
        }
        if (not short_keys.empty()) {
          this->setArg(short_keys, values);
          short_keys.clear();
          values.clear();
        }
        if (key.empty() && short_keys.empty()) {
          for (const auto& j : arg.substr(1)) {
            short_keys.push_back(j);
          }
          if (i == (argc - 1)) {
            this->setArg(short_keys, {});
          }
          continue;
        }
      } else {
        if (key.empty() && short_keys.empty()) {
          throw InvalidArgument(std::format("No keys specified"));
        }
        values.push_back(arg);
        if (i == argc - 1) {
          if (!key.empty()) {
            this->setArg(key, values);
          } else if (!short_keys.empty()) {
            this->setArg(short_keys, values);
          }
        }
      }
    }
    this->parsed_ = true;
  }

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
};

}  // namespace Argo
