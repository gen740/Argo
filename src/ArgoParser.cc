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
export using ::Argo::NULLCHAR;  // TODO(gen740) Delete

struct Unspecified {};

/*!
 * convert const char[] to std::array<char, N>
 */
export template <std::size_t N>
consteval auto key(const char (&a)[N]) -> ArgName<N - 1> {
  return {a};
}

/*!
 * convert const char[] to std::array<char, N>
 */
export template <std::size_t N>
consteval auto key(const char (&a)[N], char short_name) -> ArgName<N - 1> {
  return {a, short_name};
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

auto splitStringView(std::string_view str, char delimeter) -> std::vector<std::string_view> {
  std::vector<std::string_view> ret;
  while (str.contains(delimeter)) {
    auto pos = str.find(delimeter);
    ret.push_back(str.substr(0, pos));
    str = str.substr(pos + 1);
  }
  ret.push_back(str);
  return ret;
}

export template <auto ID = 0, class Args = std::tuple<>, class PositionalArg = void,
                 bool HelpEnabled = false>
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

  template <class Type, auto Name, auto arg1 = Unspecified(), auto arg2 = Unspecified(), class... T>
  auto createArg(T... args) {
    static constexpr auto nargs = []() {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg1)>, NArgs>) {
        return arg1;
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg2)>, NArgs>) {
        return arg2;
      } else {
        return NArgs('?');
      }
    }();

    static constexpr auto required = []() {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg1)>, bool>) {
        return arg1;
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg2)>, bool>) {
        return arg2;
      } else {
        return false;
      }
    }();

    if constexpr (!std::is_same_v<PositionalArgument, void>) {
      static_assert(!(std::string_view(Name) == std::string_view(PositionalArgument::name)),
                    "Duplicated name");
    }

    static_assert((Name.shortName == NULLCHAR) ||
                      (SearchIndexFromShortName<Arguments, Name.shortName>::value == -1),
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

    ArgInitializer<Type, Name, nargs, required, ID>::init(std::forward<T>(args)...);
    return std::type_identity<Arg<Type, Name, nargs, required, ID>>();
  }

  /*!
   * Type: type of argument
   * Name: name of argument
   * arg1: ShortName or NArgs or Unspecified
   * arg2: NArgs or Unspecified
   */
  template <auto Name, class Type, auto arg1 = Unspecified(), auto arg2 = Unspecified(), class... T>
  auto addArg(T... args) {
    auto arg = createArg<Type, Name, arg1, arg2>(std::forward<T>(args)...);
    return Parser<ID,
                  decltype(std::tuple_cat(                                      //
                      std::declval<Arguments>(),                                //
                      std::declval<std::tuple<typename decltype(arg)::type>>()  //
                      )),
                  PositionalArgument, HelpEnabled>();
  }

  template <auto Name, class Type, auto arg1 = Unspecified(), auto arg2 = Unspecified(), class... T>
  auto addPositionalArg(T... args) {
    static_assert(std::is_same_v<PositionalArg, void>,
                  "Positional argument cannot set more than one");
    static_assert(Name.shortName == NULLCHAR, "Positional argment cannot have short name");
    auto arg = createArg<Type, Name, arg1, arg2>(std::forward<T>(args)...);
    return Parser<ID, Arguments, typename decltype(arg)::type, HelpEnabled>();
  }

  template <auto Name, class... T>
  auto addFlag(T... args) {
    if constexpr (!std::is_same_v<PositionalArgument, void>) {
      static_assert(!(std::string_view(Name) == std::string_view(PositionalArgument::name)),
                    "Duplicated name");
    }
    static_assert((Name.shortName == NULLCHAR) ||
                      (SearchIndexFromShortName<Arguments, Name.shortName>::value == -1),
                  "Duplicated short name");
    static_assert(                                        //
        Argo::SearchIndex<Arguments, Name>::value == -1,  //
        "Duplicated name");

    FlagArgInitializer<Name, ID>::init(std::forward<T>(args)...);
    return Parser<ID,
                  decltype(std::tuple_cat(                           //
                      std::declval<Arguments>(),                     //
                      std::declval<std::tuple<FlagArg<Name, ID>>>()  //
                      )),
                  PositionalArgument, HelpEnabled>();
  }

  auto addHelp() {
    static_assert((SearchIndexFromShortName<Arguments, 'h'>::value == -1), "Duplicated short name");
    static_assert(Argo::SearchIndex<Arguments, key("help")>::value == -1, "Duplicated name");
    return Parser<ID, Arguments, PositionalArgument, true>();
  }

  template <auto Name>
  auto getArg() -> decltype(auto) {
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
    if constexpr (HelpEnabled) {
      if (key == "help") {
        std::println("{}", formatHelp());
        std::exit(0);
      }
    }
    Assigner<Arguments, PositionalArgument>::assign(key, val);
  }

  auto setArg(std::span<char> key, std::span<std::string_view> val) const {
    if constexpr (HelpEnabled) {
      for (const auto& i : key) {
        if (i == 'h') {
          std::println("{}", formatHelp());
          std::exit(0);
        }
      }
    }
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

  std::vector<ArgInfo> getArgInfo() const {
    return HelpGenerator<Arguments>::generate();
  }

  std::string formatHelp() const {
    std::string help;
    auto helpInfo = this->getArgInfo();
    std::size_t maxFlagLength = 0;
    for (const auto& option : helpInfo) {
      if (maxFlagLength < option.name.size()) {
        maxFlagLength = option.name.size();
      }
    }
    help.append("Options:");
    for (const auto& option : helpInfo) {
      help.push_back('\n');
      auto description = splitStringView(option.description, '\n');

      help.append(std::format(                                                             //
          "  {} --{} {} {}",                                                               //
          (option.shortName == NULLCHAR) ? "   " : std::format("-{},", option.shortName),  //
          option.name,                                                                     //
          std::string(maxFlagLength - option.name.size(), ' '),                            //
          description[0]                                                                   //
          ));
      for (std::size_t i = 1; i < description.size(); i++) {
        help.push_back('\n');
        help.append(std::format(              //
            "      {}    {}",                 //
            std::string(maxFlagLength, ' '),  //
            description[i]                    //
            ));
      }

      // erase trailing spaces
      auto pos = help.find_last_not_of(' ');
      help = help.substr(0, pos + 1);
    }
    return help;
  }
};

}  // namespace Argo
