module;

export module Argo:Initializer;

import :Validation;
import :ArgName;
import :Arg;

import :std_module;

// generator start here

namespace Argo {

using namespace std;

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
  string_view description;
};

export constexpr auto description(string_view desc) -> Description {
  return {.description = desc};
}

export template <class T>
constexpr auto explicitDefault(T value) -> ExlicitDefaultValue<T> {
  return {.explicit_default_value = value};
}

export template <class T>
constexpr auto implicitDefault(T value) -> ImplicitDefaultValue<T> {
  return {.implicit_default_value = value};
}

template <class Type, ArgName Name, NArgs nargs, bool Required, ParserID ID>
struct ArgInitializer {
  template <class Head, class... Tails>
  static auto init(Head head, Tails... tails) {
    using Arg = Arg<Type, Name, nargs, Required, ID>;
    if constexpr (is_same_v<Head, Description>) {
      Arg::description = head.description;
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      Validation::ValidationBase>) {
      static_assert(is_invocable_v<decltype(head), typename Arg::type,
                                   span<string_view>, string_view>,
                    "Invalid validator");
      Arg::validator = head;
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      ImplicitDefaultValueTag>) {
      Arg::defaultValue = static_cast<Type>(head.implicit_default_value);
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      ExplicitDefaultValueTag>) {
      Arg::value = static_cast<Type>(head.explicit_default_value);
    } else if constexpr (is_invocable_v<Head, typename Arg::type&,
                                        span<string_view>>) {
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

template <ArgName Name, ParserID ID>
struct FlagArgInitializer {
  template <class Head, class... Tails>
  static auto init(Head head, Tails... tails) {
    using FlagArg = FlagArg<Name, ID>;
    if constexpr (is_same_v<Head, Description>) {
      FlagArg::description = head.description;
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      Validation::ValidationBase>) {
      static_assert(false, "Flag cannot have validator");
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      ImplicitDefaultValueTag>) {
      static_assert(false, "Flag cannot have implicit default value");
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      ExplicitDefaultValueTag>) {
      static_assert(false, "Flag cannot have explicit default value");
    } else if constexpr (is_invocable_v<Head>) {
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

// generator end here
