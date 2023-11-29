module;

export module Argo:Initializer;

import :Validation;
import :NArgs;
import :Arg;
import :std_module;

namespace Argo {

struct ExplicitDefaultValueTag {};

template <class T>
struct ExlicitDefaultValue : ExplicitDefaultValueTag {
  T explicit_default_value;
};

struct ImplicitDefaultValueTag {};

template <class T>
struct ImplicitDefaultValue : ImplicitDefaultValueTag {
  T implicit_default_value;
};

struct Description {
  std::string_view description;
};

}  // namespace Argo

export namespace Argo {

constexpr auto withDescription(std::string_view description) -> Description {
  return {.description = description};
}

template <class T>
constexpr auto explicitDefault(T value) -> ExlicitDefaultValue<T> {
  return {.explicit_default_value = value};
}

template <class T>
constexpr auto implicitDefault(T value) -> ImplicitDefaultValue<T> {
  return {.implicit_default_value = value};
}

template <class Type, auto Name, NArgs nargs, bool Required, auto ID>
struct ArgInitializer {
  template <class Head, class... Tails>
  static auto init(Head head, Tails... tails) {
    using Arg = Arg<Type, Name, nargs, Required, ID>;
    if constexpr (std::is_same_v<Head, Description>) {
      Arg::description = head.description;
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           Validation::ValidationTag>) {
      Arg::validator = head;
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           ImplicitDefaultValueTag>) {
      Arg::defaultValue = static_cast<Type>(head.implicit_default_value);
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           ExplicitDefaultValueTag>) {
      Arg::value = static_cast<Type>(head.explicit_default_value);
    } else if constexpr (std::is_invocable_v<Head, typename Arg::type, std::string_view>) {
      Arg::callback = head;
    } else {
      static_assert(false, "Invalid argument");
    }
    if constexpr (sizeof...(Tails) != 0) {
      init(tails...);
    }
  }

  static auto init() {}
};

template <auto Name, auto ID>
struct FlagArgInitializer {
  template <class Head, class... Tails>
  static auto init(Head head, Tails... tails) {
    using FlagArg = FlagArg<Name, ID>;
    if constexpr (std::is_same_v<Head, Description>) {
      FlagArg::description = head.description;
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           Validation::ValidationTag>) {
      static_assert(false, "Flag cannot have validator");
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           ImplicitDefaultValueTag>) {
      static_assert(false, "Flag cannot have implicit default value");
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           ExplicitDefaultValueTag>) {
      static_assert(false, "Flag cannot have explicit default value");
    } else if constexpr (std::is_invocable_v<Head, bool, std::string_view>) {
      FlagArg::callback = head;
    } else {
      static_assert(false, "Invalid argument");
    }
    if constexpr (sizeof...(Tails) != 0) {
      init(tails...);
    }
  }

  static auto init() {}
};

};  // namespace Argo
