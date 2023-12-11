module;

export module Argo:MetaChecker;

import :Exceptions;
import :Initializer;
import :TypeTraits;
import :Arg;
import :std_module;

// generator start here

namespace Argo {

using namespace std;

/*!
 * Checking if the given argument is required key, cycle through all the
 * tuple argument.
 */
template <class Args>
inline constexpr auto RequiredChecker() {
  auto required_keys = vector<string_view>();
  [&required_keys]<class... T>(type_sequence<T...>) {
    (
        [&required_keys] {
          if ((T::required && !T::assigned)) {
            required_keys.push_back(string_view(T::name));
          }
        }(),
        ...);
  }(make_type_sequence_t<Args>());
  return required_keys;
}

/*!
 * Checking if the given argument is assigned, cycle through all the
 * tuple argument.
 */
template <class Args>
inline constexpr auto AssignChecker() {
  auto assigned_keys = vector<string_view>();
  [&assigned_keys]<class... T>(type_sequence<T...>) {
    (
        [&assigned_keys] {
          if (T::assigned) {
            assigned_keys.push_back(string_view(T::name));
          }
        }(),
        ...);
  }(make_type_sequence_t<Args>());
  return assigned_keys;
}

}  // namespace Argo

// generator end here
