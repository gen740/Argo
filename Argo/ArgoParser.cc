module;

#include "Argo/ArgoMacros.hh"

export module Argo:Parser;

import :Exceptions;
import :Initializer;
import :MetaParse;
import :TypeTraits;
import :MetaLookup;
import :ArgName;
import :Arg;
import :std_module;

// generator start here

namespace Argo {

using namespace std;

struct Unspecified {};

export enum class RequiredFlag : bool {
  Optional = false,
  Required = true,
};

export using RequiredFlag::Required;
export using RequiredFlag::Optional;

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
  optional<string_view> help = nullopt;
  optional<string_view> program_name = nullopt;
  optional<string_view> description = nullopt;
  optional<string_view> usage = nullopt;
  optional<string_view> subcommand_help = nullopt;
  optional<string_view> options_help = nullopt;
  optional<string_view> positional_argument_help = nullopt;
};

export template <ParserID ID = 0, class Args = tuple<>, class PArgs = tuple<>,
                 class HArg = void, class SubParsers = tuple<>>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
class Parser {
 private:
  bool parsed_ = false;
  unique_ptr<ParserInfo> info_ = nullptr;

 public:
  constexpr explicit Parser() : info_(make_unique<ParserInfo>()){};

  constexpr explicit Parser(string_view program_name)
      : info_(make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
  };

  constexpr explicit Parser(string_view program_name, string_view description)
      : info_(make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
    this->info_->description = description;
  };

  constexpr explicit Parser(string_view program_name,
                            Argo::Description description)
      : info_(make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
    this->info_->description = description.description;
  };

  Parser(const Parser&) = delete;
  Parser(Parser&&) = delete;

  SubParsers subParsers;

  constexpr explicit Parser(SubParsers tuple) : subParsers(tuple) {}

  constexpr explicit Parser(unique_ptr<ParserInfo> info, SubParsers tuple)
      : info_(std::move(info)), subParsers(tuple){};

  template <class Type, ArgName Name, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), bool ISPArgs, class... T>
  constexpr auto createArg(T... args) {
    static_assert(Name.hasValidNameLength(),
                  "Short name can't be more than one charactor");

    static constexpr auto nargs = []() {
      if constexpr (is_same_v<remove_cvref_t<decltype(arg1)>, NArgs>) {
        if constexpr (is_array_v<Type>) {
          static_assert(array_len_v<Type> == arg1.nargs,
                        "Array size mismatch with nargs");
        }
        if constexpr (is_vector_v<Type>) {
          static_assert(arg1.nargs_char != '?' && arg1.nargs != 1,
                        "Vector size mismatch with nargs");
        }
        if constexpr (is_tuple_v<Type>) {
          static_assert(tuple_size_v<Type> == arg1.nargs,
                        "Tuple size mismatch with nargs");
        }
        return arg1;
      } else if constexpr (is_same_v<remove_cvref_t<decltype(arg2)>, NArgs>) {
        if constexpr (is_array_v<Type>) {
          static_assert(array_len_v<Type> == arg2.nargs,
                        "Array size mismatch with nargs");
        }
        if constexpr (is_vector_v<Type>) {
          static_assert(arg2.nargs_char != '?' && arg2.nargs != 1,
                        "Vector size mismatch with nargs");
        }
        if constexpr (is_tuple_v<Type>) {
          static_assert(tuple_size_v<Type> == arg2.nargs,
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
          return NArgs{static_cast<int>(tuple_size_v<Type>)};
        }
        if constexpr (ISPArgs) {
          return NArgs(1);
        }
        return NArgs('?');
      }
    }();

    static_assert(!(is_array_v<Type> and nargs.nargs == 1),
                  "Array size must be more than one");
    static_assert(!(is_tuple_v<Type> and nargs.nargs == 1),
                  "Tuple size must be more than one");

    static constexpr auto required = []() {
      if constexpr (is_same_v<remove_cvref_t<decltype(arg1)>, RequiredFlag>) {
        return static_cast<bool>(arg1);
      } else if constexpr (is_same_v<remove_cvref_t<decltype(arg2)>,
                                     RequiredFlag>) {
        return static_cast<bool>(arg2);
      } else {
        return false;
      }
    }();
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      static_assert(SearchIndex<PArgs, Name>() == -1, "Duplicated name");
    }
    static_assert(
        (Name.getShortName() == '\0') ||
            (SearchIndexFromShortName<Args, Name.getShortName()>() == -1),
        "Duplicated short name");
    static_assert(SearchIndex<Args, Name>() == -1, "Duplicated name");
    static_assert(                     //
        (nargs.nargs > 0               //
         || nargs.nargs_char == '?'    //
         || nargs.nargs_char == '+'    //
         || nargs.nargs_char == '*'),  //
        "nargs must be '?', '+', '*' or int");

    ArgInitializer<Type, Name, nargs, required, ID>(std::forward<T>(args)...);
    return type_identity<Arg<Type, Name, nargs, required, ID>>();
  }

  /*!
   * Name: name of argument
   * Type: type of argument
   * arg1: Required(bool) or NArgs or Unspecified
   * arg2: Required(bool) or NArgs or Unspecified
   */
  template <ArgName Name, class Type, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), class... T>
  constexpr auto addArg(T... args) {
    auto arg =
        createArg<Type, Name, arg1, arg2, false>(std::forward<T>(args)...);
    return Parser<ID, tuple_append_t<Args, typename decltype(arg)::type>, PArgs,
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
  constexpr auto addPositionalArg(T... args) {
    static_assert(Name.getShortName() == '\0',
                  "Positional argment cannot have short name");
    auto arg =
        createArg<Type, Name, arg1, arg2, true>(std::forward<T>(args)...);

    static_assert(decltype(arg)::type::nargs.nargs_char != '?',
                  "Cannot assign narg: ? to the positional argument");
    static_assert(decltype(arg)::type::nargs.nargs_char != '*',
                  "Cannot assign narg: * to the positional argument");

    return Parser<ID, Args, tuple_append_t<PArgs, typename decltype(arg)::type>,
                  HArg, SubParsers>(std::move(this->info_), subParsers);
  }

  template <ArgName Name, class... T>
  constexpr auto addFlag(T... args) {
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      static_assert(SearchIndex<PArgs, Name>() == -1, "Duplicated name");
    }
    static_assert(
        (Name.getShortName() == '\0') ||
            (SearchIndexFromShortName<Args, Name.getShortName()>() == -1),
        "Duplicated short name");
    static_assert(SearchIndex<Args, Name>() == -1, "Duplicated name");
    FlagArgInitializer<Name, ID>(std::forward<T>(args)...);
    return Parser<ID, tuple_append_t<Args, FlagArg<Name, ID>>, PArgs, HArg,
                  SubParsers>(std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  constexpr auto addHelp() {
    static_assert((SearchIndexFromShortName<Args, Name.getShortName()>() == -1),
                  "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>() == -1, "Duplicated name");
    return Parser<ID, Args, PArgs, HelpArg<Name, ID>, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  constexpr auto addHelp(string_view help) {
    static_assert((SearchIndexFromShortName<Args, Name.getShortName()>() == -1),
                  "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>() == -1, "Duplicated name");
    static_assert(Name.hasValidNameLength(),
                  "Short name can't be more than one charactor");
    this->info_->help = help;
    return Parser<ID, Args, PArgs, HelpArg<Name, ID>, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name>
  constexpr auto getArg() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      if constexpr (SearchIndex<PArgs, Name>() != -1) {
        return tuple_element_t<SearchIndex<PArgs, Name>(), PArgs>::value;
      } else {
        static_assert(SearchIndex<Args, Name>() != -1,
                      "Argument does not exist");
        return remove_cvref_t<decltype(get<SearchIndex<Args, Name>()>(
            declval<Args>()))>::value;
      }
    } else {
      static_assert(SearchIndex<Args, Name>() != -1, "Argument does not exist");
      return remove_cvref_t<decltype(get<SearchIndex<Args, Name>()>(
          declval<Args>()))>::value;
    }
  }

  template <ArgName Name>
  constexpr auto& getParser() {
    if constexpr (is_same_v<SubParsers, tuple<>>) {
      static_assert(false, "Parser has no sub parser");
    }
    static_assert(!(SearchIndex<SubParsers, Name>() == -1),
                  "Could not find subparser");
    return get<SearchIndex<SubParsers, Name>()>(subParsers).parser.get();
  }

  template <ArgName Name>
  constexpr auto isAssigned() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      if constexpr (Name.getKey() == PArgs::name.getKey()) {
        return PArgs::assigned;
      } else {
        return remove_cvref_t<decltype(get<SearchIndex<Args, Name>()>(
            declval<Args>()))>::assigned;
      }
    } else {
      return remove_cvref_t<decltype(get<SearchIndex<Args, Name>()>(
          declval<Args>()))>::assigned;
    }
  }

  /*!
   * Add subcommand
   */
  template <ArgName Name, class T>
  ARGO_ALWAYS_INLINE constexpr auto addParser(T& sub_parser,
                                              Description description = {""}) {
    auto s = make_tuple(
        SubParser<Name, T>{ref(sub_parser), description.description});
    auto sub_parsers = tuple_cat(subParsers, s);
    return Parser<ID, Args, PArgs, HArg, decltype(sub_parsers)>(
        std::move(this->info_), sub_parsers);
  }

  ARGO_ALWAYS_INLINE constexpr auto resetArgs() -> void;

  ARGO_ALWAYS_INLINE constexpr auto addUsageHelp(string_view usage) {
    this->info_->usage = usage;
  }

  ARGO_ALWAYS_INLINE constexpr auto addSubcommandHelp(
      string_view subcommand_help) {
    this->info_->subcommand_help = subcommand_help;
  }

  ARGO_ALWAYS_INLINE constexpr auto addPositionalArgumentHelp(
      string_view positional_argument_help) {
    this->info_->positional_argument_help = positional_argument_help;
  }

  ARGO_ALWAYS_INLINE constexpr auto addOptionsHelp(string_view options_help) {
    this->info_->options_help = options_help;
  }

 private:
  ARGO_ALWAYS_INLINE constexpr auto setArg(string_view key,
                                           span<string_view> val) const -> void;
  ARGO_ALWAYS_INLINE constexpr auto setArg(span<char> key,
                                           span<string_view> val) const -> void;

 public:
  ARGO_ALWAYS_INLINE constexpr auto parse(int argc, char* argv[]) -> void;
  [[nodiscard]] constexpr string formatHelp(bool no_color = false) const;

  explicit constexpr operator bool() const {
    return this->parsed_;
  }
};

}  // namespace Argo

// generator end here
