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

/*!
 * Helper class of assigning value
 */
template <class Type>
ARGO_ALWAYS_INLINE constexpr auto ArgCaster(const std::string_view& value,
                                            const std::string_view& key)
    -> Type {
  if constexpr (std::is_same_v<Type, bool>) {
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
    throw InvalidArgument(
        format("Argument {}: {} cannot convert bool", key, value));
  } else if constexpr (std::is_integral_v<Type>) {
    Type ret;
    std::from_chars(value.begin(), value.end(), ret);
    return ret;
  } else if constexpr (std::is_floating_point_v<Type>) {
    return static_cast<Type>(std::stod(std::string(value)));
  } else if constexpr (std::is_same_v<Type, const char*>) {
    return value.data();
  } else {
    return static_cast<Type>(value);
  }
}

template <class... T, size_t... N>
ARGO_ALWAYS_INLINE constexpr auto TupleAssign(
    std::tuple<T...>& t, const std::span<std::string_view>& v,
    std::index_sequence<N...> /* unused */, const std::string_view& key)
    -> void {
  ((get<N>(t) = ArgCaster<std::remove_cvref_t<decltype(get<N>(t))>>(v[N], key)),
   ...);
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto AfterAssign(
    const std::span<std::string_view>& values) -> void {
  Arg::assigned = true;
  if (Arg::validator) {
    Arg::validator(Arg::value, values, Arg::name.getKey());
  }
  if (Arg::callback) {
    Arg::callback(Arg::value, values);
  }
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto ValiadicArgAssign(
    const std::span<std::string_view>& values) -> void {
  Arg::value.resize(values.size());
  for (size_t i = 0; i < values.size(); i++) {
    Arg::value[i] =
        ArgCaster<typename Arg::baseType>(values[i], Arg::name.getKey());
  }
  AfterAssign<Arg>(values);
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto NLengthArgAssign(
    std::span<std::string_view>& values) -> void {
  if (Arg::nargs.nargs > values.size()) [[unlikely]] {
    throw Argo::InvalidArgument(
        format("Argument {}: invalid argument {}", Arg::name.getKey(), values));
  }
  if constexpr (is_array_v<typename Arg::type>) {
    for (size_t i = 0; i < Arg::nargs.nargs; i++) {
      Arg::value[i] =
          ArgCaster<typename Arg::baseType>(values[i], Arg::name.getKey());
    }
  } else if constexpr (is_vector_v<typename Arg::type>) {
    Arg::value.resize(Arg::nargs.nargs);
    for (size_t i = 0; i < Arg::nargs.nargs; i++) {
      Arg::value[i] =
          ArgCaster<typename Arg::baseType>(values[i], Arg::name.getKey());
    }
  } else if constexpr (is_tuple_v<typename Arg::type>) {
    TupleAssign(
        Arg::value, values,
        std::make_index_sequence<std::tuple_size_v<typename Arg::type>>(),
        Arg::name.getKey());
  } else {
    static_assert(false, "Invalid Type");
  }
  AfterAssign<Arg>(values.subspan(0, Arg::nargs.nargs));
  values = values.subspan(Arg::nargs.nargs);
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto ZeroOrOneArgAssign(
    std::span<std::string_view>& values) -> void {
  if (values.empty()) {
    Arg::value = Arg::defaultValue;
  } else {
    Arg::value = ArgCaster<typename Arg::type>(values[0], Arg::name.getKey());
  }
  AfterAssign<Arg>(values.subspan(0, 1));
  values = values.subspan(1);
}

template <class PArgs>
ARGO_ALWAYS_INLINE constexpr auto PArgAssigner(
    std::span<std::string_view> values) -> bool {
  return [&values]<class... Arg>(type_sequence<Arg...>) ARGO_ALWAYS_INLINE {
    return ([&values] ARGO_ALWAYS_INLINE {
      if (Arg::assigned) {
        return false;
      }
      if constexpr (Arg::nargs.nargs_char == '+') {
        ValiadicArgAssign<Arg>(values);
        return true;
      }
      if constexpr (Arg::nargs.nargs == 1) {
        if (values.empty()) [[unlikely]] {
          throw Argo::InvalidArgument(
              format("Argument {}: should take exactly one value but zero",
                     Arg::name.getKey()));
        }
        ZeroOrOneArgAssign<Arg>(values);
        return values.empty();
      }
      if constexpr (Arg::nargs.nargs > 1) {
        NLengthArgAssign<Arg>(values);
        return values.empty();
      }
    }() || ...);
  }(make_type_sequence_t<PArgs>());
}

template <>
ARGO_ALWAYS_INLINE constexpr auto PArgAssigner<std::tuple<>>(
    std::span<std::string_view> /*unused*/) -> bool {
  return true;
}

template <class Head, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto AssignOneArg(
    const std::string_view& key, std::span<std::string_view> values) -> bool {
  if (Head::assigned) [[unlikely]] {
    throw Argo::InvalidArgument(
        format("Argument {}: duplicated argument", key));
  }
  if constexpr (std::derived_from<Head, FlagArgTag>) {
    if constexpr (std::is_same_v<PArgs, std::tuple<>>) {
      if (!values.empty()) [[unlikely]] {
        throw Argo::InvalidArgument(format("Flag {} can not take value", key));
      }
    } else {
      if (!values.empty()) {
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
    if constexpr (Head::nargs.nargs_char == '?') {
      ZeroOrOneArgAssign<Head>(values);
      if (values.empty()) {
        return true;
      }
      return PArgAssigner<PArgs>(values);
    } else if constexpr (Head::nargs.nargs_char == '*') {
      if (values.empty()) {
        Head::value = Head::defaultValue;
        Head::assigned = true;
        return true;
      }
      ValiadicArgAssign<Head>(values);
      return true;
    } else if constexpr (Head::nargs.nargs_char == '+') {
      if (values.empty()) [[unlikely]] {
        throw Argo::InvalidArgument(
            format("Argument {}: should take more than one value", key));
      }
      ValiadicArgAssign<Head>(values);
      return true;
    } else if constexpr (Head::nargs.nargs == 1) {
      if (values.empty()) [[unlikely]] {
        throw Argo::InvalidArgument(
            format("Argument {}: should take exactly one value but zero", key));
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
ARGO_ALWAYS_INLINE constexpr auto assignArg(
    const std::string_view& key, const std::span<std::string_view>& values) {
  [&key, &values]<size_t... Is>(std::index_sequence<Is...>)
      ARGO_ALWAYS_INLINE -> void {
        if (!(... || (std::tuple_element_t<Is, Args>::name.getKey() == key and
                      AssignOneArg<std::tuple_element_t<Is, Args>, PArgs>(
                          key, values)))) [[unlikely]] {
          throw Argo::InvalidArgument(format("Invalid argument {}", key));
        }
      }(std::make_index_sequence<std::tuple_size_v<Args>>());
}

template <class Arguments, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto Assigner(
    std::string_view key, const std::span<std::string_view>& values) -> void {
  if constexpr (!std::is_same_v<PArgs, std::tuple<>>) {
    if (key.empty()) {
      if (!PArgAssigner<PArgs>(values)) [[unlikely]] {
        throw InvalidArgument("Duplicated positional argument");
      }
      return;
    }
  } else {
    if (key.empty()) [[unlikely]] {
      throw Argo::InvalidArgument(format("Invalid argument {}", key));
    }
  }
  assignArg<Arguments, PArgs>(key, values);
}

template <class Arguments, class PArgs, class HArg>
ARGO_ALWAYS_INLINE constexpr auto ShortArgAssigner(
    std::string_view key, const std::span<std::string_view>& values) {
  for (size_t i = 0; i < key.size(); i++) {
    auto [found_key, is_flag] = GetkeyFromShortKey<     //
        std::conditional_t<std::is_same_v<HArg, void>,  //
                           Arguments,                   //
                           tuple_append_t<Arguments, HArg>>>(key[i]);
    if constexpr (!std::is_same_v<HArg, void>) {
      if (found_key == HArg::name.getKey()) [[unlikely]] {
        return true;
      }
    }
    if (is_flag and (key.size() - 1 == i) and !values.empty()) {
      assignArg<Arguments, PArgs>(found_key, values);
    } else if (is_flag) {
      assignArg<Arguments, PArgs>(found_key, {});
    } else if ((key.size() - 1 == i) and !values.empty()) {
      assignArg<Arguments, PArgs>(found_key, values);
      return false;
    } else if ((key.size() - 1 == i) and values.empty()) {
      auto value = std::vector<std::string_view>{key.substr(i + 1)};
      assignArg<Arguments, PArgs>(found_key, value);
      return false;
    } else [[unlikely]] {
      throw Argo::InvalidArgument(
          format("Invalid Flag argument {} {}", key[i], key.substr(i + 1)));
    }
  }
  return false;
}

template <class Args>
ARGO_ALWAYS_INLINE constexpr auto ValueReset() -> void {
  []<size_t... Is>(std::index_sequence<Is...>) ARGO_ALWAYS_INLINE {
    (..., []<class T>() ARGO_ALWAYS_INLINE {
      if (T::assigned) {
        T::value = typename T::type();
        T::assigned = false;
      }
    }.template operator()<std::tuple_element_t<Is, Args>>());
  }(std::make_index_sequence<std::tuple_size_v<Args>>());
}

};  // namespace Argo

// generator end here
