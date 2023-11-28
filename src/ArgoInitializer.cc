module;

export module Argo:Initializer;

import :Validation;
import :NArgs;
import :Arg;
import :std_module;

namespace Argo {

struct DefaultValueTag {};

template <class T>
struct DefaultValue : DefaultValueTag {
  T default_value;
};

}  // namespace Argo

export namespace Argo {

struct withDescription {
 private:
  std::string_view description_;

 public:
  explicit withDescription(std::string_view description) : description_(description) {}

  auto getDescription() {
    return this->description_;
  }
};

template <class T>
auto defaultValue(T value) -> DefaultValue<T> {
  return {.default_value = value};
}

template <class Type, auto Name, char ShortName, NArgs nargs, bool Required, int ID>
struct ArgInitializer {
  template <class Head, class... Tails>
  static auto init(Head head, Tails... tails) {
    using Arg = Arg<Type, Name, ShortName, nargs, Required, ID>;
    if constexpr (std::is_same_v<Head, withDescription>) {
      Arg::description = head.getDescription();
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           Validation::ValidationTag>) {
      Arg::validator = head;
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           DefaultValueTag>) {
      Arg::defaultValue = static_cast<Type>(head.default_value);
      // } else if constexpr (std::is_invocable_v<Head, typename Head::type>) {
      //   Head::callback = [head](std::string_view, Head::type value) { head(value); };
    } else if constexpr (std::is_invocable_v<Head, std::string_view, typename Arg::type>) {
      Arg::callback = [head](std::string_view key, Arg::type value) { head(key, value); };
      // } else if constexpr (std::is_invocable_v<Head, std::string_view>) {
      //   Head::callback = [head](std::string_view key, Head::type) { head(key); };
    } else {
      static_assert(false, "Invalid argument");
    }
    if constexpr (sizeof...(Tails) != 0) {
      init(tails...);
    }
  }

  static auto init() {}
};

template <auto Name, char ShortName, int ID>
struct FlagArgInitializer {
  template <class Head, class... Tails>
  static auto init(Head head, Tails... tails) {
    if constexpr (std::is_same_v<Head, withDescription>) {
      FlagArg<Name, ShortName, ID>::description = head.getDescription();
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           Validation::ValidationTag>) {
      static_assert(false, "Cannot assign validator to the Flag");
    } else {
      static_assert(false, "Cannot assign default value to the Flag");
    }
    if constexpr (sizeof...(Tails) != 0) {
      init(tails...);
    }
  }

  static auto init() {}
};

};  // namespace Argo
