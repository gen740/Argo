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
auto MetaParse(SubParsers sub_parsers, std::string_view key, int argc, char** argv) -> bool {
  return std::apply(
      [&](auto&&... s) {
        return (... || (s.name == key && (s.parser.get().parse(argc, argv), true)));
      },
      sub_parsers);
};

}  // namespace Argo
