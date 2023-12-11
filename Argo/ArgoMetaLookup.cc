module;

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
constexpr inline auto GetNameFromShortName(char key) {
  auto name = string_view();
  if ([&name, &key]<class... T>(type_sequence<T...>) {
        return ([&name, &key] {
          if (T::name.shortName == key) {
            name = string_view(T::name);
            return true;
          }
          return false;
        }() || ...);
      }(make_type_sequence_t<Arguments>())) {
    return name;
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
        return ((value++, C == T::name.shortName) || ...);
      }(make_type_sequence_t<Tuple>())) {
    return -1;
  }
  return value;
}

};  // namespace Argo

// generator end here
