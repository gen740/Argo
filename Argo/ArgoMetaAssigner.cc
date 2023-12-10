module;

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
constexpr auto caster(const string_view& value) -> Type {
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
constexpr auto tupleAssign(tuple<T...>& t, span<string_view> v,
                           index_sequence<N...> /* unused */) {
  ((get<N>(t) = caster<remove_cvref_t<decltype(get<N>(t))>>(v[N])), ...);
}

template <class Arg>
auto assignValiadicArg(const span<string_view>& values) {
  for (const auto& value : values) {
    Arg::value.push_back(caster<typename Arg::baseType>(value));
  }
  if (Arg::validator) {
    Arg::validator(Arg::value, values, string_view(Arg::name));
  }
  if (Arg::callback) {
    Arg::callback(Arg::value, values);
  }
  Arg::assigned = true;
}

template <class Arg>
auto assignNLengthArg(span<string_view>& values) {
  if (Arg::nargs.getNargs() > values.size()) {
    throw Argo::InvalidArgument(format("Argument {}: invalid argument {}",
                                       string_view(Arg::name), values));
  }
  if constexpr (is_array_v<typename Arg::type>) {
    for (int i = 0; i < Arg::nargs.getNargs(); i++) {
      Arg::value[i] = caster<typename Arg::baseType>(values[i]);
    }
  } else if constexpr (is_vector_v<typename Arg::type>) {
    for (int i = 0; i < Arg::nargs.getNargs(); i++) {
      Arg::value.push_back(caster<typename Arg::baseType>(values[i]));
    }
  } else if constexpr (is_tuple_v<typename Arg::type>) {
    tupleAssign(Arg::value, values,
                make_index_sequence<tuple_size_v<typename Arg::type>>());
  } else {
    static_assert(false, "Invalid Type");
  }

  if (Arg::validator) {
    Arg::validator(Arg::value, values.subspan(0, Arg::nargs.getNargs()),
                   string_view(Arg::name));
  }
  if (Arg::callback) {
    Arg::callback(Arg::value, values.subspan(0, Arg::nargs.getNargs()));
  }
  Arg::assigned = true;
  values = values.subspan(Arg::nargs.getNargs());
}

template <class Arg>
auto assignZeroOrOneArg(span<string_view>& values) {
  if (values.empty()) {
    Arg::value = Arg::defaultValue;
    Arg::assigned = true;
    return;
  }
  Arg::value = caster<typename Arg::type>(values[0]);
  Arg::assigned = true;
  if (Arg::validator) {
    Arg::validator(Arg::value, values, string_view(Arg::name));
  }
  if (Arg::callback) {
    Arg::callback(Arg::value, values);
  }
  values = values.subspan(1);
  return;
}

template <class PArgs>
struct PArgAssigner {};

template <class... PArgs>
struct PArgAssigner<tuple<PArgs...>> {
  static auto assign(span<string_view> values) {
    return ([]<ArgType Arg>(auto& values) {
      if (Arg::assigned) {
        return false;
      }
      if constexpr (Arg::nargs.getNargsChar() == '+') {
        assignValiadicArg<Arg>(values);
        return true;
      }
      if constexpr (Arg::nargs.getNargs() > 0) {
        assignNLengthArg<Arg>(values);
        return values.empty();
      }
    }.template operator()<PArgs>(values) ||
            ...);
  }
};

template <>
struct PArgAssigner<std::tuple<>> {
  static auto assign(span<string_view> /*unused*/) {
    return true;
  }
};

template <class Arguments, class PArgs>
struct Assigner {
  template <ArgType Head>
  static constexpr auto assignOneArg(const string_view& key,
                                     span<string_view> values) -> bool {
    if constexpr (derived_from<Head, FlagArgTag>) {
      if (!values.empty()) {
        if constexpr (is_same_v<PArgs, tuple<>>) {
          throw Argo::InvalidArgument(
              format("Flag {} can not take value", key));
        } else {
          PArgAssigner<PArgs>::assign(values);
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
        assignZeroOrOneArg<Head>(values);
        if (values.empty()) {
          return true;
        }
        return PArgAssigner<PArgs>::assign(values);
      } else if constexpr (Head::nargs.getNargsChar() == '*') {
        if (values.empty()) {
          Head::value = Head::defaultValue;
          Head::assigned = true;
          return true;
        }
        assignValiadicArg<Head>(values);
        return true;
      } else if constexpr (Head::nargs.getNargsChar() == '+') {
        if (values.empty()) {
          throw Argo::InvalidArgument(
              format("Argument {} should take more than one value", key));
        }
        assignValiadicArg<Head>(values);
        return true;
      } else if constexpr (Head::nargs.getNargs() == 1) {
        if (values.empty()) {
          throw Argo::InvalidArgument(format(
              "Argument {} should take exactly one value but zero", key));
        }
        assignZeroOrOneArg<Head>(values);
        if (values.empty()) {
          return true;
        }
        return PArgAssigner<PArgs>::assign(values);
      } else {
        assignNLengthArg<Head>(values);
        if (values.empty()) {
          return true;
        }
        return PArgAssigner<PArgs>::assign(values);
      }
    }
    return false;
  }

  template <class Args>
  static auto assignImpl([[maybe_unused]] const string_view& key,
                         [[maybe_unused]] const span<string_view>& values) {
    [&key, &values]<size_t... Is>(index_sequence<Is...> /*unused*/) {
      if (!(... || (string_view(tuple_element_t<Is, Args>::name) == key and
                    assignOneArg<tuple_element_t<Is, Args>>(key, values)))) {
        throw Argo::InvalidArgument(format("Invalid argument {}", key));
      }
    }(make_index_sequence<tuple_size_v<Args>>());
  }

  template <class T>
  static auto assignFlagImpl(string_view key) -> void {
    [&key]<size_t... Is>(index_sequence<Is...>) {
      if (!(... || [&key]<ArgType Head>() {
            if constexpr (derived_from<Head, FlagArgTag>) {
              if (string_view(Head::name) == key) {
                Head::value = true;
                return true;
              }
              return false;
            } else {
              return false;
            }
          }.template operator()<tuple_element_t<Is, T>>())) {
        throw Argo::InvalidArgument("");
      }
    }(make_index_sequence<tuple_size_v<T>>());
  }

  static auto assign(string_view key, span<string_view> values) -> void {
    if (key.empty()) {
      if constexpr (!is_same_v<PArgs, tuple<>>) {
        if (!PArgAssigner<PArgs>::assign(values)) {
          throw InvalidArgument(format("Duplicated positional argument"));
        }
        return;
      } else {
        throw Argo::InvalidArgument(
            format("Assigner: Invalid argument {}", key));
      }
    }
    assignImpl<Arguments>(key, values);
  };

  // Multiple key assigner

  static auto assign(span<char> key, span<string_view> values) {
    for (size_t i = 0; i < key.size() - 1; i++) {
      assignFlagImpl<Arguments>(GetNameFromShortName<Arguments>::eval(key[i]));
    }
    assignImpl<Arguments>(GetNameFromShortName<Arguments>::eval(key.back()),
                          values);
  };
};

template <class Args>
auto ValueReset() {
  []<size_t... Is>(index_sequence<Is...>) {
    (..., []<ArgType T>() {
      if (T::assigned) {
        T::value = typename T::type();
        T::assigned = false;
      }
    }.template operator()<tuple_element_t<Is, Args>>());
  }(make_index_sequence<tuple_size_v<Args>>());
}

};  // namespace Argo

// generator end here
