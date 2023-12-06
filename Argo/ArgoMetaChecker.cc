module;

export module Argo:MetaChecker;

import :Exceptions;
import :Initializer;
import :NArgs;
import :Arg;
import :std_module;

// generator start here

export namespace Argo {

/*!
 * Checking if the given argument is required key, cycle through all the
 * tuple argument.
 */
template <class Args>
struct RequiredChecker {};

template <ArgType... Args>
struct RequiredChecker<std::tuple<Args...>> {
  static auto check() -> std::vector<std::string_view> {
    auto required_keys = std::vector<std::string_view>();
    (
        [&required_keys]<class T>() {
          if constexpr (std::derived_from<T, ArgTag>) {
            if (T::required && !T::assigned) {
              required_keys.push_back(std::string_view(T::name));
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
struct AssignChecker<std::tuple<Args...>> {
  static auto check() -> std::vector<std::string_view> {
    auto assigned_keys = std::vector<std::string_view>();
    (
        [&assigned_keys]<class T>() {
          if constexpr (std::derived_from<T, ArgTag>) {
            if (T::assigned) {
              assigned_keys.push_back(std::string_view(T::name));
            }
          }
        }.template operator()<Args>(),
        ...);
    return assigned_keys;
  };
};

}  // namespace Argo

// generator end here
