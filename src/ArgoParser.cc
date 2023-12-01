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

namespace Argo {

struct Unspecified {};

export enum class RequiredFlag : bool {
  optional = false,
  required = true,
};

export using RequiredFlag::required;  // NOLINT(misc-unused-using-decls)
export using RequiredFlag::optional;  // NOLINT(misc-unused-using-decls)

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

export template <ParserID ID = 0, class Args = std::tuple<>, class PositionalArg = void,
                 class SubParserTuple = std::tuple<>, bool HelpEnabled = false>
class Parser {
 private:
  bool parsed_ = false;
  std::string_view programName_;

 public:
  constexpr explicit Parser() = default;
  constexpr explicit Parser(std::string_view programName) : programName_(programName){};
  Parser(const Parser&) = delete;
  Parser(Parser&&) = delete;

  using Arguments = Args;
  using PositionalArgument = PositionalArg;
  SubParserTuple subParsers;

  constexpr explicit Parser(SubParserTuple tuple) : subParsers(tuple) {}

  template <class Type, ArgName Name, auto arg1 = Unspecified(), auto arg2 = Unspecified(),
            class... T>
  auto createArg(T... args) {
    static_assert(!Name.containsInvalidChar(), "Name has invalid char");
    static_assert(Name.hasValidNameLength(), "Short name can't be more than one charactor");

    static constexpr auto nargs = []() {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg1)>, NArgs>) {
        if constexpr (is_array_v<Type>) {
          static_assert(array_len_v<Type> == arg1.nargs, "Array size mismatch with nargs");
        }
        if constexpr (is_vector_v<Type>) {
          static_assert(arg1.nargs_char != '?' && arg1.nargs != 1,
                        "Vector size mismatch with nargs");
        }
        return arg1;
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg2)>, NArgs>) {
        if constexpr (is_array_v<Type>) {
          static_assert(array_len_v<Type> == arg1.nargs, "Array size mismatch with nargs");
        }
        if constexpr (is_vector_v<Type>) {
          static_assert(arg1.nargs_char != '?' && arg1.nargs != 1,
                        "Vector size mismatch with nargs");
        }
        return arg2;
      } else {
        if constexpr (is_array_v<Type>) {
          return NArgs(static_cast<int>(array_len_v<Type>));
        }
        if constexpr (is_vector_v<Type>) {
          return NArgs('*');
        }
        if constexpr (is_tuple_v<Type>) {
          return NArgs(static_cast<int>(std::tuple_size_v<Type>));
        }
        return NArgs('?');
      }
    }();

    static constexpr auto required = []() {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg1)>, RequiredFlag>) {
        return static_cast<bool>(arg1);
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg2)>, RequiredFlag>) {
        return static_cast<bool>(arg2);
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
  template <ArgName Name, class Type, auto arg1 = Unspecified(), auto arg2 = Unspecified(),
            class... T>
  auto addArg(T... args) {
    auto arg = createArg<Type, Name, arg1, arg2>(std::forward<T>(args)...);
    return Parser<ID,
                  decltype(std::tuple_cat(                                      //
                      std::declval<Arguments>(),                                //
                      std::declval<std::tuple<typename decltype(arg)::type>>()  //
                      )),
                  PositionalArgument, SubParserTuple, HelpEnabled>();
  }

  template <ArgName Name, class Type, auto arg1 = Unspecified(), auto arg2 = Unspecified(),
            class... T>
  auto addPositionalArg(T... args) {
    static_assert(std::is_same_v<PositionalArg, void>,
                  "Positional argument cannot set more than one");
    static_assert(Name.shortName == NULLCHAR, "Positional argment cannot have short name");
    auto arg = createArg<Type, Name, arg1, arg2>(std::forward<T>(args)...);
    return Parser<ID, Arguments, typename decltype(arg)::type, SubParserTuple, HelpEnabled>();
  }

  template <ArgName Name, class... T>
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
                  PositionalArgument, SubParserTuple, HelpEnabled>();
  }

  auto addHelp() {
    static_assert((SearchIndexFromShortName<Arguments, 'h'>::value == -1), "Duplicated short name");
    static_assert(Argo::SearchIndex<Arguments, "help">::value == -1, "Duplicated name");
    return Parser<ID, Arguments, PositionalArgument, SubParserTuple, true>();
  }

  template <ArgName Name>
  auto getArg() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!std::is_same_v<PositionalArgument, void>) {
      if constexpr (std::string_view(Name) == std::string_view(PositionalArgument::name)) {
        return PositionalArgument::value;
      } else {
        return std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(
            std::declval<Arguments>()))>::value;
      }
    } else {
      return std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(
          std::declval<Arguments>()))>::value;
    }
  }

  template <ArgName Name>
  auto& getParser() {
    if constexpr (std::is_same_v<SubParserTuple, std::tuple<>>) {
      static_assert(false, "Parser has no sub parser");
    }
    static_assert(!(SearchIndex<SubParserTuple, Name>::value == -1), "Could not find subparser");
    return std::get<SearchIndex<SubParserTuple, Name>::value>(subParsers).parser.get();
  }

  template <ArgName Name>
  auto isAssigned() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!std::is_same_v<PositionalArgument, void>) {
      if constexpr (std::string_view(Name) == std::string_view(PositionalArgument::name)) {
        return PositionalArgument::assigned;
      } else {
        return std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(
            std::declval<Arguments>()))>::assigned;
      }
    } else {
      return std::remove_cvref_t<decltype(std::get<SearchIndex<Arguments, Name>::value>(
          std::declval<Arguments>()))>::assigned;
    }
  }

  /*!
   * Add subcommand
   */
  template <ArgName Name, class T>
  auto addParser(T& sub_parser, Description description = {""}) {
    auto s = std::make_tuple(                                              //
        SubParser<Name, T>{std::ref(sub_parser), description.description}  //
    );
    auto sub_parsers = std::tuple_cat(subParsers, s);
    return Parser<ID,                     //
                  Arguments,              //
                  PositionalArgument,     //
                  decltype(sub_parsers),  //
                  HelpEnabled>(sub_parsers);
  }

  auto resetArgs() -> void;

 private:
  auto setArg(std::string_view key, std::span<std::string_view> val) const -> void;
  auto setArg(std::span<char> key, std::span<std::string_view> val) const -> void;

 public:
  auto parse(int argc, char* argv[]) -> void;
  std::string formatHelp() const;

  operator bool() const {
    return this->parsed_;
  }
};

}  // namespace Argo
