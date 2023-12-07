module;

export module Argo:Parser;

import :Exceptions;
import :Initializer;
import :MetaParse;
import :TypeTraits;
import :MetaLookup;
import :Arg;
import :NArgs;
import :std_module;

// generator start here

namespace Argo {

struct Unspecified {};

export enum class RequiredFlag : bool {
  optional = false,
  required = true,
};

export using RequiredFlag::required;
export using RequiredFlag::optional;

/*!
 * Helper function to create nargs
 */
export consteval auto nargs(char narg) -> NArgs {
  return NArgs(narg);
}

export consteval auto nargs(int narg) -> NArgs {
  return NArgs(narg);
}

struct ParserInfo {
  std::optional<std::string_view> help = std::nullopt;
  std::optional<std::string_view> program_name = std::nullopt;
  std::optional<std::string_view> description = std::nullopt;
  std::optional<std::string_view> usage = std::nullopt;
  std::optional<std::string_view> subcommand_help = std::nullopt;
  std::optional<std::string_view> options_help = std::nullopt;
  std::optional<std::string_view> positional_argument_help = std::nullopt;
};

export template <ParserID ID = 0, class Args = std::tuple<>,
                 class PArg = std::tuple<>, class HArg = void,
                 class SubParsers = std::tuple<>>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
class Parser {
 private:
  bool parsed_ = false;
  std::unique_ptr<ParserInfo> info_ = nullptr;

 public:
  constexpr explicit Parser() : info_(std::make_unique<ParserInfo>()){};

  constexpr explicit Parser(std::string_view program_name)
      : info_(std::make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
  };

  constexpr explicit Parser(std::string_view program_name,
                            std::string_view description)
      : info_(std::make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
    this->info_->description = description;
  };

  constexpr explicit Parser(std::string_view program_name,
                            Argo::Description description)
      : info_(std::make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
    this->info_->description = description.description;
  };

  Parser(const Parser&) = delete;
  Parser(Parser&&) = delete;

  SubParsers subParsers;

  constexpr explicit Parser(SubParsers tuple) : subParsers(tuple) {}

  constexpr explicit Parser(std::unique_ptr<ParserInfo> info, SubParsers tuple)
      : info_(std::move(info)), subParsers(tuple){};

  template <class Type, ArgName Name, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), bool ISPArg, class... T>
  auto createArg(T... args) {
    static_assert(!Name.containsInvalidChar(), "Name has invalid char");
    static_assert(Name.hasValidNameLength(),
                  "Short name can't be more than one charactor");

    static constexpr auto nargs = []() {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg1)>,
                                   NArgs>) {
        if constexpr (is_array_v<Type>) {
          static_assert(array_len_v<Type> == arg1.nargs,
                        "Array size mismatch with nargs");
        }
        if constexpr (is_vector_v<Type>) {
          static_assert(arg1.nargs_char != '?' && arg1.nargs != 1,
                        "Vector size mismatch with nargs");
        }
        if constexpr (is_tuple_v<Type>) {
          static_assert(std::tuple_size_v<Type> == arg1.nargs,
                        "Tuple size mismatch with nargs");
        }
        return arg1;
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg2)>,
                                          NArgs>) {
        if constexpr (is_array_v<Type>) {
          static_assert(array_len_v<Type> == arg2.nargs,
                        "Array size mismatch with nargs");
        }
        if constexpr (is_vector_v<Type>) {
          static_assert(arg2.nargs_char != '?' && arg2.nargs != 1,
                        "Vector size mismatch with nargs");
        }
        if constexpr (is_tuple_v<Type>) {
          static_assert(std::tuple_size_v<Type> == arg2.nargs,
                        "Tuple size mismatch with nargs");
        }
        return arg2;
      } else {
        if constexpr (is_array_v<Type>) {
          return NArgs{static_cast<int>(array_len_v<Type>)};
        }
        if constexpr (is_vector_v<Type>) {
          return NArgs{'*'};
        }
        if constexpr (is_tuple_v<Type>) {
          return NArgs{static_cast<int>(std::tuple_size_v<Type>)};
        }
        if constexpr (ISPArg) {
          return NArgs(1);
        }
        return NArgs('?');
      }
    }();

    static_assert(!(is_array_v<Type> and nargs.getNargs() == 1),
                  "Array size must be more than one");
    static_assert(!(is_tuple_v<Type> and nargs.getNargs() == 1),
                  "Tuple size must be more than one");

    static constexpr auto required = []() {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg1)>,
                                   RequiredFlag>) {
        return static_cast<bool>(arg1);
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg2)>,
                                          RequiredFlag>) {
        return static_cast<bool>(arg2);
      } else {
        return false;
      }
    }();
    if constexpr (!std::is_same_v<PArg, std::tuple<>>) {
      static_assert(SearchIndex<PArg, Name>::value == -1, "Duplicated name");
    }
    static_assert(
        (Name.shortName == '\0') ||
            (SearchIndexFromShortName<Args, Name.shortName>::value == -1),
        "Duplicated short name");
    static_assert(                                   //
        Argo::SearchIndex<Args, Name>::value == -1,  //
        "Duplicated name");
    static_assert(                         //
        (nargs.getNargs() > 0              //
         || nargs.getNargsChar() == '?'    //
         || nargs.getNargsChar() == '+'    //
         || nargs.getNargsChar() == '*'),  //
        "nargs must be '?', '+', '*' or int");

    ArgInitializer<Type, Name, nargs, required, ID>::init(
        std::forward<T>(args)...);
    return std::type_identity<Arg<Type, Name, nargs, required, ID>>();
  }

  /*!
   * Name: name of argument
   * Type: type of argument
   * arg1: Required(bool) or NArgs or Unspecified
   * arg2: Required(bool) or NArgs or Unspecified
   */
  template <ArgName Name, class Type, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), class... T>
  auto addArg(T... args) {
    auto arg =
        createArg<Type, Name, arg1, arg2, false>(std::forward<T>(args)...);
    return Parser<ID, tuple_append_t<Args, typename decltype(arg)::type>, PArg,
                  HArg, SubParsers>(std::move(this->info_), subParsers);
  }

  /*!
   * Name: name of argument
   * Type: type of argument
   * arg1: Required(bool) or NArgs or Unspecified
   * arg2: Required(bool) or NArgs or Unspecified
   */
  template <ArgName Name, class Type, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), class... T>
  auto addPositionalArg(T... args) {
    static_assert(Name.shortName == '\0',
                  "Positional argment cannot have short name");
    auto arg =
        createArg<Type, Name, arg1, arg2, true>(std::forward<T>(args)...);

    static_assert(decltype(arg)::type::nargs.getNargsChar() != '?',
                  "Cannot assign narg: ? to the positional argument");
    static_assert(decltype(arg)::type::nargs.getNargsChar() != '*',
                  "Cannot assign narg: * to the positional argument");

    return Parser<ID, Args, tuple_append_t<PArg, typename decltype(arg)::type>,
                  HArg, SubParsers>(std::move(this->info_), subParsers);
  }

  template <ArgName Name, class... T>
  auto addFlag(T... args) {
    if constexpr (!std::is_same_v<PArg, std::tuple<>>) {
      static_assert(SearchIndex<PArg, Name>::value == -1, "Duplicated name");
    }
    static_assert(
        (Name.shortName == '\0') ||
            (SearchIndexFromShortName<Args, Name.shortName>::value == -1),
        "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>::value == -1,
                  "Duplicated name");
    FlagArgInitializer<Name, ID>::init(std::forward<T>(args)...);
    return Parser<ID, tuple_append_t<Args, FlagArg<Name, ID>>, PArg, HArg,
                  SubParsers>(std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  auto addHelp() {
    static_assert((SearchIndexFromShortName<Args, Name.shortName>::value == -1),
                  "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>::value == -1,
                  "Duplicated name");
    return Parser<ID, Args, PArg, HelpArg<Name, ID>, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  auto addHelp(std::string_view help) {
    static_assert((SearchIndexFromShortName<Args, Name.shortName>::value == -1),
                  "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>::value == -1,
                  "Duplicated name");
    static_assert(!Name.containsInvalidChar(), "Name has invalid char");
    static_assert(Name.hasValidNameLength(),
                  "Short name can't be more than one charactor");
    this->info_->help = help;
    return Parser<ID, Args, PArg, HelpArg<Name, ID>, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name>
  auto getArg() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!std::is_same_v<PArg, std::tuple<>>) {
      if constexpr (SearchIndex<PArg, Name>::value != -1) {
        return std::tuple_element_t<SearchIndex<PArg, Name>::value,
                                    PArg>::value;
      } else {
        static_assert(SearchIndex<Args, Name>::value != -1,
                      "Argument does not exist");
        return std::remove_cvref_t<
            decltype(std::get<SearchIndex<Args, Name>::value>(
                std::declval<Args>()))>::value;
      }
    } else {
      static_assert(SearchIndex<Args, Name>::value != -1,
                    "Argument does not exist");
      return std::remove_cvref_t<
          decltype(std::get<SearchIndex<Args, Name>::value>(
              std::declval<Args>()))>::value;
    }
  }

  template <ArgName Name>
  auto& getParser() {
    if constexpr (std::is_same_v<SubParsers, std::tuple<>>) {
      static_assert(false, "Parser has no sub parser");
    }
    static_assert(!(SearchIndex<SubParsers, Name>::value == -1),
                  "Could not find subparser");
    return std::get<SearchIndex<SubParsers, Name>::value>(subParsers)
        .parser.get();
  }

  template <ArgName Name>
  auto isAssigned() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!std::is_same_v<PArg, std::tuple<>>) {
      if constexpr (std::string_view(Name) == std::string_view(PArg::name)) {
        return PArg::assigned;
      } else {
        return std::remove_cvref_t<
            decltype(std::get<SearchIndex<Args, Name>::value>(
                std::declval<Args>()))>::assigned;
      }
    } else {
      return std::remove_cvref_t<
          decltype(std::get<SearchIndex<Args, Name>::value>(
              std::declval<Args>()))>::assigned;
    }
  }

  /*!
   * Add subcommand
   */
  template <ArgName Name, class T>
  auto addParser(T& sub_parser, Description description = {""}) {
    auto s = std::make_tuple(
        SubParser<Name, T>{std::ref(sub_parser), description.description});
    auto sub_parsers = std::tuple_cat(subParsers, s);
    return Parser<ID, Args, PArg, HArg, decltype(sub_parsers)>(
        std::move(this->info_), sub_parsers);
  }

  auto resetArgs() -> void;

  auto addUsageHelp(std::string_view usage) {
    this->info_->usage = usage;
  }

  auto addSubcommandHelp(std::string_view subcommand_help) {
    this->info_->subcommand_help = subcommand_help;
  }

  auto addPositionalArgumentHelp(std::string_view positional_argument_help) {
    this->info_->positional_argument_help = positional_argument_help;
  }

  auto addOptionsHelp(std::string_view options_help) {
    this->info_->options_help = options_help;
  }

 private:
  auto setArg(std::string_view key,
              std::span<std::string_view> val) const -> void;
  auto setArg(std::span<char> key,
              std::span<std::string_view> val) const -> void;

 public:
  auto parse(int argc, char* argv[]) -> void;
  [[nodiscard]] std::string formatHelp(bool no_color = false) const;

  explicit operator bool() const {
    return this->parsed_;
  }
};

}  // namespace Argo

// generator end here
