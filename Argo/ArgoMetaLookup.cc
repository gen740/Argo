module;

#include "Argo/ArgoMacros.hh"

export module Argo:MetaLookup;

import std;

import :Exceptions;
import :TypeTraits;
import :Arg;
import :ArgName;

// generator start here

namespace Argo {

template <class Arguments>
ARGO_ALWAYS_INLINE constexpr auto GetkeyFromShortKey(char key)
    -> std::tuple<std::string_view, bool> {
  auto name = std::string_view();
  auto is_flag = true;
  if ([&]<class... T>(type_sequence<T...>) ARGO_ALWAYS_INLINE {
        return ([&] {
          if (T::name.getShortName() == key) {
            name = T::name.getKey();
            if constexpr (!std::derived_from<T, FlagArgTag>) {
              is_flag = false;
            }
            return true;
          }
          return false;
        }() || ...);
      }(make_type_sequence_t<Arguments>())) [[likely]] {
    return std::make_tuple(name, is_flag);
  }
  throw ParserInternalError("Fail to lookup");
}

/*!
 * Index Search meta function
 */
template <class Tuple, ArgName Name>
consteval auto SearchIndex() -> int {
  int value = -1;
  if ([&value]<class... T>(type_sequence<T...>) ARGO_ALWAYS_INLINE {
        return ((value++, Name == T::name) || ...);
      }(make_type_sequence_t<Tuple>())) {
    return value;
  }
  return -1;
}

/*!
 * Index Search meta function
 */
template <class Tuple, char C>
consteval auto SearchIndexFromShortName() -> int {
  int value = -1;
  if ([&value]<class... T>(type_sequence<T...>) ARGO_ALWAYS_INLINE {
        return ((value++, C == T::name.getShortName()) || ...);
      }(make_type_sequence_t<Tuple>())) {
    return value;
  }
  return -1;
}

/*!
 * Index Search meta function
 */
template <class Tuple>
ARGO_ALWAYS_INLINE constexpr auto IsFlag(char c) -> bool {
  return [&c]<class... T>(type_sequence<T...>) ARGO_ALWAYS_INLINE {
    return ((c == T::name.getShortName()) || ...);
  }(make_type_sequence_t<Tuple>());
}

};  // namespace Argo

// generator end here
