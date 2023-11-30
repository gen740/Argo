module;

export module Argo:MetaParse;
import :std_module;
import :Arg;

export namespace Argo {

template <ArgName Name, class Parser>
struct SubParser {
  static constexpr auto name = Name;
  std::reference_wrapper<Parser> parser;
  std::string_view description;
};

template <class SubParsers>
struct MetaParser {
  template <int Index, class Head, class... Tails>
  struct ParseImpl {};

  template <int Index>
  struct ParseImpl<Index, std::tuple<>> {
    static auto eval(SubParsers /*unused*/, std::string_view /*unused*/, int /*unused*/,
                     char** /*unused*/) {
      return false;
    }
  };

  template <int Index, class Head, class... Tails>
  struct ParseImpl<Index, std::tuple<Head, Tails...>> {
    static auto eval(SubParsers sub_parsers, std::string_view key, int argc, char** argv) -> bool {
      if (key == Head::name) {
        std::get<Index>(sub_parsers).parser.get().parse(argc, argv);
        return true;
      }
      return ParseImpl<1 + Index, std::tuple<Tails...>>::eval(sub_parsers, key, argc, argv);
    }
  };

  static auto parse(SubParsers sub_parsers, std::string_view key, int argc, char** argv) -> bool {
    return ParseImpl<0, SubParsers>::eval(sub_parsers, key, argc, argv);
  };
};

}  // namespace Argo
