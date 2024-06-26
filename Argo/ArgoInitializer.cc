module;

#include "Argo/ArgoMacros.hh"

export module Argo:Initializer;

import :Validation;
import :ArgName;
import :Arg;

import std;

// generator start here

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

export ARGO_ALWAYS_INLINE constexpr auto description(std::string_view desc)
    -> Description {
  return {.description = desc};
}

export template <class T>
ARGO_ALWAYS_INLINE constexpr auto explicitDefault(T value)
    -> ExlicitDefaultValue<T> {
  return {.explicit_default_value = value};
}

export template <class T>
ARGO_ALWAYS_INLINE constexpr auto implicitDefault(T value)
    -> ImplicitDefaultValue<T> {
  return {.implicit_default_value = value};
}

template <class Type, ArgName Name, NArgs nargs, bool Required, ParserID ID,
          class... Args>
ARGO_ALWAYS_INLINE constexpr auto ArgInitializer(Args... args) -> void {
  (
      [&args]() ARGO_ALWAYS_INLINE {
        using Arg = Arg<Type, Name, nargs, Required, ID>;
        if constexpr (std::is_same_v<Args, Description>) {
          Arg::description = args.description;
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               Validation::ValidationBase>) {
          static_assert(std::is_invocable_v<Args, typename Arg::type,
                                            std::span<std::string_view>,
                                            std::string_view>,
                        "Invalid validator");
          Arg::validator = args;
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               ImplicitDefaultValueTag>) {
          Arg::defaultValue = static_cast<Type>(args.implicit_default_value);
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               ExplicitDefaultValueTag>) {
          Arg::value = static_cast<Type>(args.explicit_default_value);
        } else if constexpr (std::is_invocable_v<Args, typename Arg::type&,
                                                 std::span<std::string_view>>) {
          Arg::callback = args;
        } else {
          static_assert(false, "Invalid argument");
        }
      }(),
      ...);
}

template <ArgName Name, ParserID ID, class... Args>
ARGO_ALWAYS_INLINE constexpr auto FlagArgInitializer(Args... args) -> void {
  (
      [&args]() ARGO_ALWAYS_INLINE {
        using FlagArg = FlagArg<Name, ID>;
        if constexpr (std::is_same_v<Args, Description>) {
          FlagArg::description = args.description;
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               Validation::ValidationBase>) {
          static_assert(false, "Flag cannot have validator");
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               ImplicitDefaultValueTag>) {
          static_assert(false, "Flag cannot have implicit default value");
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               ExplicitDefaultValueTag>) {
          static_assert(false, "Flag cannot have explicit default value");
        } else if constexpr (std::is_invocable_v<Args>) {
          FlagArg::callback = args;
        } else {
          static_assert(false, "Invalid argument");
        }
      }(),
      ...);
}

};  // namespace Argo

// generator end here
