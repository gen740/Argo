module;

export module Argo:MetaChecker;

import :Exceptions;
import :Initializer;
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
struct RequiredChecker {};

template <ArgType... Args>
struct RequiredChecker<tuple<Args...>> {
  static auto check() -> vector<string_view> {
    auto required_keys = vector<string_view>();
    (
        [&required_keys]<class T>() {
          if constexpr (derived_from<T, ArgTag>) {
            if (T::required && !T::assigned) {
              required_keys.push_back(string_view(T::name));
            }
          }
        }.template operator()<Args>(),
        ...);
    return required_keys;
  };
};

/*!
 * Checking if the given argument is assigned, cycle through all the
 * tuple argument.
 */
template <class Args>
struct AssignChecker {};

template <ArgType... Args>
struct AssignChecker<tuple<Args...>> {
  static auto check() -> vector<string_view> {
    auto assigned_keys = vector<string_view>();
    (
        [&assigned_keys]<class T>() {
          if constexpr (derived_from<T, ArgTag>) {
            if (T::assigned) {
              assigned_keys.push_back(string_view(T::name));
            }
          }
        }.template operator()<Args>(),
        ...);
    return assigned_keys;
  };
};

}  // namespace Argo

// generator end here
