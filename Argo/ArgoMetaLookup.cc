module;

#include "Argo/ArgoMacros.hh"

export module Argo:MetaLookup;

import :Exceptions;
import :std_module;
import :TypeTraits;
import :Arg;
import :ArgName;

// generator start here

namespace Argo {

using namespace std;

template <class Arguments>
ARGO_ALWAYS_INLINE constexpr auto GetkeyFromShortKey(char key) {
  auto name = string_view();
  auto is_flag = true;
  if ([&]<class... T>(type_sequence<T...>) ARGO_ALWAYS_INLINE {
        return ([&] {
          if (T::name.getShortName() == key) {
            name = T::name.getKey();
            if constexpr (!derived_from<T, FlagArgTag>) {
              is_flag = false;
            }
            return true;
          }
          return false;
        }() || ...);
      }(make_type_sequence_t<Arguments>())) {
    return make_tuple(name, is_flag);
  }
  throw ParserInternalError("Fail to lookup");
}

/*!
 * Index Search meta function
 */
template <class Tuple, ArgName Name>
consteval auto SearchIndex() {
  int value = -1;
  if (![&value]<class... T>(type_sequence<T...>) {
        return ((value++, Name == T::name) || ...);
      }(make_type_sequence_t<Tuple>())) {
    return -1;
  }
  return value;
}

/*!
 * Index Search meta function
 */
template <class Tuple, char C>
consteval auto SearchIndexFromShortName() {
  int value = -1;
  if (![&value]<class... T>(type_sequence<T...>) {
        return ((value++, C == T::name.getShortName()) || ...);
      }(make_type_sequence_t<Tuple>())) {
    return -1;
  }
  return value;
}

/*!
 * Index Search meta function
 */
template <class Tuple>
ARGO_ALWAYS_INLINE constexpr auto SearchIndexFromShortName(char c) {
  int value = -1;
  if (![&value, &c]<class... T>(type_sequence<T...>) ARGO_ALWAYS_INLINE {
        return ((value++, (T::name.getShortName() >= '0' and
                           T::name.getShortName() <= '9') and
                              (c == T::name.getShortName())) ||
                ...);
      }(make_type_sequence_t<Tuple>())) {
    return -1;
  }
  return value;
}

};  // namespace Argo

// generator end here
