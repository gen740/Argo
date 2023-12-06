module;

export module Argo:MetaParse;
import :std_module;
import :Arg;

// generator start here

export namespace Argo {

template <ArgName Name, class Parser>
struct SubParser {
  static constexpr auto name = Name;
  std::reference_wrapper<Parser> parser;
  std::string_view description;
};

template <class SubParsers>
auto MetaParse(SubParsers sub_parsers, int index, int argc,
               char** argv) -> bool {
  return std::apply(
      [&](auto&&... s) {
        std::int64_t idx = -1;
        return (
            ... ||
            (idx++, idx == index && (s.parser.get().parse(argc, argv), true)));
      },
      sub_parsers);
};

template <class SubParsers>
constexpr auto ParserIndex(SubParsers sub_parsers,  //
                           std::string_view key) -> std::int64_t {
  return std::apply(
      [&](auto&&... s) {
        std::int64_t index = -1;
        bool found = (... || (index++, s.name == key));
        return found ? index : -1;
      },
      sub_parsers);
};

}  // namespace Argo

// generator end here
