module;

#include "Argo/ArgoMacros.hh"

export module Argo:MetaAssigner;

import :Exceptions;
import :Validation;
import :TypeTraits;
import :MetaLookup;
import :Arg;
import :std_module;

// generator start here

namespace Argo {

using namespace std;

/*!
 * Helper class of assigning value
 */
template <class Type>
ARGO_ALWAYS_INLINE constexpr auto ArgCaster(const string_view& value) -> Type {
  if constexpr (is_same_v<Type, bool>) {
    if ((value == "true")     //
        || (value == "True")  //
        || (value == "TRUE")  //
        || (value == "1")) {
      return true;
    }
    if ((value == "false")     //
        || (value == "False")  //
        || (value == "FALSE")  //
        || (value == "0")) {
      return false;
    }
    throw InvalidArgument("Invalid argument expect bool");
  } else if constexpr (is_integral_v<Type>) {
    Type ret;
    from_chars(value.begin(), value.end(), ret);
    return ret;
  } else if constexpr (is_floating_point_v<Type>) {
    return static_cast<Type>(stod(string(value)));
  } else if constexpr (is_same_v<Type, const char*>) {
    return value.data();
  } else {
    return static_cast<Type>(value);
  }
}

template <class... T, size_t... N>
ARGO_ALWAYS_INLINE constexpr auto TupleAssign(tuple<T...>& t,
                                              span<string_view> v,
                                              index_sequence<N...> /* unused */)
    -> void {
  ((get<N>(t) = ArgCaster<remove_cvref_t<decltype(get<N>(t))>>(v[N])), ...);
}

template <ArgType Arg>
ARGO_ALWAYS_INLINE constexpr auto AfterAssign(const span<string_view>& values)
    -> void {
  Arg::assigned = true;
  if (Arg::validator) {
    Arg::validator(Arg::value, values, string_view(Arg::name));
  }
  if (Arg::callback) {
    Arg::callback(Arg::value, values);
  }
}

template <ArgType Arg>
ARGO_ALWAYS_INLINE constexpr auto ValiadicArgAssign(
    const span<string_view>& values) -> void {
  Arg::value.resize(values.size());
  for (size_t i = 0; i < values.size(); i++) {
    Arg::value[i] = ArgCaster<typename Arg::baseType>(values[i]);
  }
  AfterAssign<Arg>(values);
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto NLengthArgAssign(span<string_view>& values)
    -> void {
  if (Arg::nargs.getNargs() > values.size()) {
    throw Argo::InvalidArgument(format("Argument {}: invalid argument {}",
                                       string_view(Arg::name), values));
  }
  if constexpr (is_array_v<typename Arg::type>) {
    for (size_t i = 0; i < Arg::nargs.getNargs(); i++) {
      Arg::value[i] = ArgCaster<typename Arg::baseType>(values[i]);
    }
  } else if constexpr (is_vector_v<typename Arg::type>) {
    Arg::value.resize(Arg::nargs.getNargs());
    for (size_t i = 0; i < Arg::nargs.getNargs(); i++) {
      Arg::value[i] = ArgCaster<typename Arg::baseType>(values[i]);
    }
  } else if constexpr (is_tuple_v<typename Arg::type>) {
    TupleAssign(Arg::value, values,
                make_index_sequence<tuple_size_v<typename Arg::type>>());
  } else {
    static_assert(false, "Invalid Type");
  }
  AfterAssign<Arg>(values.subspan(0, Arg::nargs.getNargs()));
  values = values.subspan(Arg::nargs.getNargs());
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto ZeroOrOneArgAssign(span<string_view>& values)
    -> void {
  if (values.empty()) {
    Arg::value = Arg::defaultValue;
  } else {
    Arg::value = ArgCaster<typename Arg::type>(values[0]);
  }
  AfterAssign<Arg>(values.subspan(0, 1));
  values = values.subspan(1);
}

template <class PArgs>
ARGO_ALWAYS_INLINE constexpr auto PArgAssigner(span<string_view> values)
    -> bool {
  return [&values]<class... Arg>(type_sequence<Arg...>) ARGO_ALWAYS_INLINE {
    return ([&values] ARGO_ALWAYS_INLINE {
      if (Arg::assigned) {
        return false;
      }
      if constexpr (Arg::nargs.getNargsChar() == '+') {
        ValiadicArgAssign<Arg>(values);
        return true;
      }
      if constexpr (Arg::nargs.getNargs() > 0) {
        NLengthArgAssign<Arg>(values);
        return values.empty();
      }
    }() || ...);
  }(make_type_sequence_t<PArgs>());
}

template <>
ARGO_ALWAYS_INLINE constexpr auto PArgAssigner<std::tuple<>>(
    span<string_view> /*unused*/) -> bool {
  return true;
}

template <class Head, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto AssignOneArg(const string_view& key,
                                               span<string_view> values)
    -> bool {
  if constexpr (derived_from<Head, FlagArgTag>) {
    if (!values.empty()) {
      if constexpr (is_same_v<PArgs, tuple<>>) {
        throw Argo::InvalidArgument(format("Flag {} can not take value", key));
      } else {
        PArgAssigner<PArgs>(values);
      }
    }
    Head::value = true;
    Head::assigned = true;
    if (Head::callback) {
      Head::callback();
    }
    return true;
  } else {
    if constexpr (Head::nargs.getNargsChar() == '?') {
      ZeroOrOneArgAssign<Head>(values);
      if (values.empty()) {
        return true;
      }
      return PArgAssigner<PArgs>(values);
    } else if constexpr (Head::nargs.getNargsChar() == '*') {
      if (values.empty()) {
        Head::value = Head::defaultValue;
        Head::assigned = true;
        return true;
      }
      ValiadicArgAssign<Head>(values);
      return true;
    } else if constexpr (Head::nargs.getNargsChar() == '+') {
      if (values.empty()) {
        throw Argo::InvalidArgument(
            format("Argument {} should take more than one value", key));
      }
      ValiadicArgAssign<Head>(values);
      return true;
    } else if constexpr (Head::nargs.getNargs() == 1) {
      if (values.empty()) {
        throw Argo::InvalidArgument(
            format("Argument {} should take exactly one value but zero", key));
      }
      ZeroOrOneArgAssign<Head>(values);
      if (values.empty()) {
        return true;
      }
      return PArgAssigner<PArgs>(values);
    } else {
      NLengthArgAssign<Head>(values);
      if (values.empty()) {
        return true;
      }
      return PArgAssigner<PArgs>(values);
    }
  }
  return false;
}

template <class Args, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto assignArg(const string_view& key,
                                            const span<string_view>& values) {
  [&key, &values]<size_t... Is>(index_sequence<Is...>)
      ARGO_ALWAYS_INLINE -> void {
        if (!(... ||
              (string_view(tuple_element_t<Is, Args>::name) == key and
               AssignOneArg<tuple_element_t<Is, Args>, PArgs>(key, values)))) {
          throw Argo::InvalidArgument(format("Invalid argument {}", key));
        }
      }(make_index_sequence<tuple_size_v<Args>>());
}

template <class Arguments, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto Assigner(string_view key,
                                           span<string_view> values) -> void {
  if (key.empty()) {
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      if (!PArgAssigner<PArgs>(values)) {
        throw InvalidArgument(format("Duplicated positional argument"));
      }
      return;
    } else {
      throw Argo::InvalidArgument(format("Assigner: Invalid argument {}", key));
    }
  }
  assignArg<Arguments, PArgs>(key, values);
}

template <class Arguments, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto Assigner(span<char> key,
                                           span<string_view> values) -> void {
  for (size_t i = 0; i < key.size(); i++) {
    auto found_key = GetNameFromShortName<Arguments>(key[i]);
    if (i == key.size() - 1) {
      assignArg<Arguments, PArgs>(found_key, values);
    } else {
      assignArg<Arguments, PArgs>(found_key, {});
    }
  }
}

template <class Args>
ARGO_ALWAYS_INLINE constexpr auto ValueReset() -> void {
  []<size_t... Is>(index_sequence<Is...>) ARGO_ALWAYS_INLINE {
    (..., []<ArgType T>() ARGO_ALWAYS_INLINE {
      if (T::assigned) {
        T::value = typename T::type();
        T::assigned = false;
      }
    }.template operator()<tuple_element_t<Is, Args>>());
  }(make_index_sequence<tuple_size_v<Args>>());
}

};  // namespace Argo

// generator end here
