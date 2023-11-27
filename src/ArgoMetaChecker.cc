module;

export module Argo:MetaChecker;

import :Exceptions;
import :Initializer;
import :NArgs;
import :Arg;
import :std_module;

export namespace Argo {

template <typename... Args>
struct RequiredChecker {};

template <typename... Args>
struct RequiredChecker<std::tuple<Args...>> {
  static auto check() -> std::vector<std::string_view> {
    auto invalid_keys = std::vector<std::string_view>();
    (
        [&invalid_keys]<class T>() {
          if constexpr (std::derived_from<T, ArgTag>) {
            if (T::required && !T::assigned) {
              invalid_keys.push_back(std::string_view(T::name.begin(), T::name.end()));
            }
          }
        }.template operator()<Args>(),
        ...);
    return invalid_keys;
  };
};

}  // namespace Argo
