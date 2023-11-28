module;

export module Argo:Parser;

import :Exceptions;
import :Validation;
import :Initializer;
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

/*!
 * convert const char[] to std::array<char, N>
 */
export template <std::size_t N>
consteval auto key(const char (&a)[N]) {
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

export template <int ID = 0, class Args = std::tuple<>, class PositionalArg = void>
class Parser {
 private:
  bool parsed_ = false;
  std::string_view programName_;

 public:
  constexpr explicit Parser() = default;
  constexpr explicit Parser(std::string_view programName) : programName_(programName){};

  using Arguments = Args;
  using PositionalArgument = PositionalArg;
  Args value;

  template <class Type, auto Name, auto arg1 = Unspecified(), auto arg2 = Unspecified(),
            auto arg3 = Unspecified(), class... T>
  auto createArg(T... args) {
    static constexpr char ShortName = []() {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg1)>, char>) {
        return arg1;
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg2)>, char>) {
        return arg2;
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg3)>, char>) {
        return arg3;
      } else {
        return NULLCHAR;
      }
    }();

    static constexpr auto nargs = []() {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg1)>, NArgs>) {
        return arg1;
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg2)>, NArgs>) {
        return arg2;
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg3)>, NArgs>) {
        return arg3;
      } else {
        return NArgs('?');
      }
    }();

    static constexpr auto required = []() {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg1)>, bool>) {
        return arg1;
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg2)>, bool>) {
        return arg2;
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg3)>, bool>) {
        return arg3;
      } else {
        return false;
      }
    }();

    if constexpr (!std::is_same_v<PositionalArgument, void>) {
      static_assert(!(std::string_view(Name) == std::string_view(PositionalArgument::name)),
                    "Duplicated name");
    }

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

    ArgInitializer<Type, Name, ShortName, nargs, required, ID>::init(std::forward<T>(args)...);
    return std::type_identity<Arg<Type, Name, ShortName, nargs, required, ID>>();
  }

  /*!
   * Type: type of argument
   * Name: name of argument
   * arg1: ShortName or NArgs or Unspecified
   * arg2: NArgs or Unspecified
   */
  template <class Type, auto Name, auto arg1 = Unspecified(), auto arg2 = Unspecified(),
            auto arg3 = Unspecified(), class... T>
  auto addArg(T... args) {
    auto arg = createArg<Type, Name, arg1, arg2, arg3>(std::forward<T>(args)...);
    return Parser<ID,
                  decltype(std::tuple_cat(                                      //
                      std::declval<Arguments>(),                                //
                      std::declval<std::tuple<typename decltype(arg)::type>>()  //
                      )),
                  PositionalArgument>();
  }

  template <class Type, auto Name, auto arg1 = Unspecified(), auto arg2 = Unspecified(),
            auto arg3 = Unspecified(), class... T>
  auto addPositionalArg(T... args) {
    static_assert(std::is_same_v<PositionalArg, void>,
                  "Positional argument cannot set more than one");
    auto arg = createArg<Type, Name, arg1, arg2, arg3>(std::forward<T>(args)...);
    static_assert(decltype(arg)::type::shortName == NULLCHAR,
                  "Positonal argument could not have short name");
    return Parser<ID, Arguments, typename decltype(arg)::type>();
  }

  template <auto Name, char ShortName = NULLCHAR, class... T>
  auto addFlag(T... args) {
    if constexpr (!std::is_same_v<PositionalArgument, void>) {
      static_assert(!(std::string_view(Name) == std::string_view(PositionalArgument::name)),
                    "Duplicated name");
    }
    static_assert(
        (ShortName == NULLCHAR) || (SearchIndexFromShortName<Arguments, ShortName>::value == -1),
        "Duplicated short name");
    static_assert(                                        //
        Argo::SearchIndex<Arguments, Name>::value == -1,  //
        "Duplicated name");

    FlagArgInitializer<Name, ShortName, ID>::init(std::forward<T>(args)...);
    return Parser<ID,
                  decltype(std::tuple_cat(                                      //
                      std::declval<Arguments>(),                                //
                      std::declval<std::tuple<FlagArg<Name, ShortName, ID>>>()  //
                      )),
                  PositionalArgument>();
  }

  template <auto Name>
  auto getArg() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!std::is_same_v<PositionalArgument, void>) {
      if constexpr (std::string_view(Name) == std::string_view(PositionalArgument::name)) {
        return PositionalArgument::value;
      } else {
        return std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(
            this->value))>::value;
      }
    } else {
      return std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(
          this->value))>::value;
    }
  }

  template <auto Name>
  auto isAssigend() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!std::is_same_v<PositionalArgument, void>) {
      if constexpr (std::string_view(Name) == std::string_view(PositionalArgument::name)) {
        return PositionalArgument::assigend;
      } else {
        return std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(
            this->value))>::assigend;
      }
    } else {
      return std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(
          this->value))>::assigend;
    }
  }

 private:
  auto setArg(std::string_view key, std::span<std::string_view> val) const {
    Assigner<Arguments, PositionalArgument>::assign(key, val);
  }

  auto setArg(std::span<char> key, std::span<std::string_view> val) const {
    Assigner<Arguments, PositionalArgument>::assign(key, val);
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
          } else if (!values.empty()) {
            if constexpr (!std::is_same_v<PositionalArgument, void>) {
              this->setArg(key, values);
              values.clear();
            }
          }
          if (arg.contains('=')) {
            auto equal_pos = arg.find('=');
            auto value = std::vector<std::string_view>{arg.substr(equal_pos + 1)};
            this->setArg(arg.substr(2, equal_pos - 2), value);
            continue;
          }
          key = arg.substr(2);
          if (i == (argc - 1)) {
            this->setArg(key, {});
          }
          continue;
        }
        if (!key.empty()) {
          this->setArg(key, values);
          key = "";
          values.clear();
        } else if (!short_keys.empty()) {
          this->setArg(short_keys, values);
          short_keys.clear();
          values.clear();
        } else if (!values.empty()) {
          if constexpr (!std::is_same_v<PositionalArgument, void>) {
            this->setArg(key, values);
            values.clear();
          }
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
        if constexpr (std::is_same_v<PositionalArgument, void>) {
          if (key.empty() && short_keys.empty()) {
            throw InvalidArgument(std::format("No keys specified"));
          }
        }
        values.push_back(arg);

        if (i == argc - 1) {
          if (!key.empty()) {
            this->setArg(key, values);
          } else if (!short_keys.empty()) {
            this->setArg(short_keys, values);
          } else {
            if constexpr (std::is_same_v<PositionalArgument, void>) {
              throw InvalidArgument(std::format("No keys specified"));
            } else {
              this->setArg(key, values);
            }
          }
        }
      }
    }
    auto required_keys = RequiredChecker<Arguments>::check();
    if (!required_keys.empty()) {
      throw InvalidArgument(std::format("Requried {}", required_keys));
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
