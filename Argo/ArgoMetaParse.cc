module;

export module Argo:MetaParse;
import :std_module;
import :Arg;
import :TypeTraits;
import :ArgName;

// generator start here

namespace Argo {

using namespace std;

template <ArgName Name, class Parser>
struct SubParser {
  static constexpr auto name = Name;
  reference_wrapper<Parser> parser;
  string_view description;
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
auto MetaParse(SubParsers sub_parsers, int index, int argc, char** argv)
    -> bool {
  return apply(
      [&](auto&&... s) {
        int64_t idx = -1;
        return (... || (idx++, idx == index &&
                                   (s.parser.get().parse(argc, argv), true)));
      },
      sub_parsers);
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
constexpr auto ParserIndex(SubParsers sub_parsers,  //
                           string_view key) -> int64_t {
  return apply(
      [&](auto&&... s) {
        int64_t index = -1;
        bool found = (... || (index++, s.name == key));
        return found ? index : -1;
      },
      sub_parsers);
};

}  // namespace Argo

// generator end here
