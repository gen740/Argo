module;

#include "Argo/ArgoMacros.hh"

export module Argo:MetaParse;
import :std_module;
import :Arg;
import :TypeTraits;
import :ArgName;

// generator start here

namespace Argo {

template <ArgName Name, class Parser>
struct SubParser {
  static constexpr auto name = Name;
  std::reference_wrapper<Parser> parser;
  std::string_view description;
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
ARGO_ALWAYS_INLINE constexpr auto MetaParse(SubParsers sub_parsers, int index,
                                            int argc, char** argv) -> bool {
  return apply(
      [&](auto&&... s) ARGO_ALWAYS_INLINE {
        int64_t idx = -1;
        return (... || (idx++, idx == index &&
                                   (s.parser.get().parse(argc, argv), true)));
      },
      sub_parsers);
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
ARGO_ALWAYS_INLINE constexpr auto ParserIndex(SubParsers sub_parsers,  //
                                              std::string_view key) -> int64_t {
  return apply(
      [&](auto&&... s) ARGO_ALWAYS_INLINE {
        int64_t index = -1;
        bool found = (... || (index++, s.name.getKey() == key));
        return found ? index : -1;
      },
      sub_parsers);
};

}  // namespace Argo

// generator end here
