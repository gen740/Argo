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

/*!
 * Helper class of assigning value
 */
template <class Type>
constexpr auto caster(std::string_view value) -> Type {
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
    throw ParserInternalError("Invalid argument expect bool");
  } else if constexpr (std::is_integral_v<Type>) {
    Type tmpValue;
    std::from_chars(std::begin(value), std::end(value), tmpValue);
    return tmpValue;
  } else if constexpr (std::is_floating_point_v<Type>) {
    return static_cast<Type>(std::stod(std::string(value)));
  } else if constexpr (std::is_same_v<Type, const char*>) {
    return value.data();
  } else {
    return static_cast<Type>(value);
  }
}

template <class... T, std::size_t... N>
constexpr auto tupleAssign(std::tuple<T...>& t, std::span<std::string_view> v,
                           std::index_sequence<N...> /* unused */) {
  ((std::get<N>(t) =
        caster<std::remove_cvref_t<decltype(std::get<N>(t))>>(v[N])),
   ...);
}

export template <class PArgs>
struct PArgAssigner {};

export template <class... PArgs>
struct PArgAssigner<std::tuple<PArgs...>> {
  static auto assign(std::span<std::string_view> values) {
    return ([]<ArgType Arg>(auto& values) {
      if (Arg::assigned) {
        return false;
      }
      if constexpr (Arg::nargs.getNargsChar() == '+') {
        Arg::assigned = true;
        for (const auto& value : values) {
          Arg::value.push_back(caster<typename Arg::baseType>(value));
        }
        if (Arg::validator) {
          Arg::validator(Arg::value, values, std::string_view(Arg::name));
        }
        if (Arg::callback) {
          Arg::callback(Arg::value, values);
        }
        return true;
      }
      if constexpr (Arg::nargs.getNargs() > 0) {
        if (Arg::nargs.getNargs() > values.size()) {
          throw Argo::InvalidArgument(std::format(
              "Positional Argument {} Invalid positional argument {}",
              std::string_view(Arg::name), values));
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
                      std::make_index_sequence<
                          std::tuple_size_v<typename Arg::type>>());
        } else {
          static_assert(false, "Invalid Type");
        }

        Arg::assigned = true;
        if (Arg::validator) {
          Arg::validator(Arg::value, values.subspan(0, Arg::nargs.getNargs()),
                         std::string_view(Arg::name));
        }
        if (Arg::callback) {
          Arg::callback(Arg::value, values.subspan(0, Arg::nargs.getNargs()));
        }
        values = values.subspan(Arg::nargs.getNargs());
        return values.empty();
      }
    }.template operator()<PArgs>(values) ||
            ...);
  }
};

export template <class Arguments, class PArgs>
struct Assigner {
  template <ArgType Head>
  static constexpr auto assignOneArg(
      std::string_view key, std::span<std::string_view> values) -> bool {
    if constexpr (std::derived_from<Head, FlagArgTag>) {
      if (!values.empty()) {
        if constexpr (std::is_same_v<PArgs, std::tuple<>>) {
          throw Argo::InvalidArgument(
              std::format("Flag {} can not take value", key));
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
        if (values.empty()) {
          Head::value = Head::defaultValue;
          Head::assigned = true;
          return true;
        }
        if (values.size() == 1) {
          Head::value = caster<typename Head::type>(values[0]);
          Head::assigned = true;
          if (Head::validator) {
            Head::validator(Head::value, values, key);
          }
          if (Head::callback) {
            Head::callback(Head::value, values);
          }
          return true;
        }
        if constexpr (std::is_same_v<PArgs, std::tuple<>>) {
          throw Argo::InvalidArgument(
              std::format("Argument {} cannot take more than one value got {}",
                          key, values.size()));
        } else {
          assignOneArg<Head>(key, values.subspan(0, 1));
          return PArgAssigner<PArgs>::assign(values.subspan(1));
        }
      } else if constexpr (Head::nargs.getNargsChar() == '*') {
        if (values.empty()) {
          Head::value = Head::defaultValue;
          Head::assigned = true;
          return true;
        }
        for (const auto& value : values) {
          Head::value.emplace_back(
              caster<vector_base_t<typename Head::type>>(value));
        }
        Head::assigned = true;
        if (Head::validator) {
          Head::validator(Head::value, values, key);
        }
        if (Head::callback) {
          Head::callback(Head::value, values);
        }
        return true;
      } else if constexpr (Head::nargs.getNargsChar() == '+') {
        if (values.empty()) {
          throw Argo::InvalidArgument(
              std::format("Argument {} should take more than one value", key));
        }
        for (const auto& value : values) {
          Head::value.emplace_back(caster<typename Head::baseType>(value));
        }
        Head::assigned = true;
        if (Head::validator) {
          Head::validator(Head::value, values, key);
        }
        if (Head::callback) {
          Head::callback(Head::value, values);
        }
        return true;
      } else if constexpr (Head::nargs.getNargs() == 1) {
        if (values.empty()) {
          throw Argo::InvalidArgument(std::format(
              "Argument {} should take exactly one value but zero", key));
        }
        if (values.size() > 1) {
          if constexpr (std::is_same_v<PArgs, std::tuple<>>) {
            throw Argo::InvalidArgument(
                std::format("Argument {} should take exactly one value but {}",
                            key, values.size()));
          } else {
            assignOneArg<Head>(key, values.subspan(0, 1));
            return PArgAssigner<PArgs>::assign(values.subspan(1));
          }
        }
        Head::value = caster<typename Head::baseType>(values[0]);
        Head::assigned = true;
        if (Head::validator) {
          Head::validator(Head::value, values, key);
        }
        if (Head::callback) {
          Head::callback(Head::value, values);
        }
        return true;
      } else {
        if (values.size() == Head::nargs.getNargs()) {
          if constexpr (is_array_v<typename Head::type>) {
            for (int idx = 0; idx < Head::nargs.getNargs(); idx++) {
              Head::value[idx] =
                  caster<array_base_t<typename Head::type>>(values[idx]);
            }
          } else if constexpr (is_tuple_v<typename Head::type>) {
            tupleAssign(Head::value, values,
                        std::make_index_sequence<
                            std::tuple_size_v<typename Head::type>>());
          } else {
            for (const auto& value : values) {
              Head::value.emplace_back(
                  caster<vector_base_t<typename Head::type>>(value));
            }
          }

          Head::assigned = true;
          if (Head::validator) {
            Head::validator(Head::value, values, key);
          }
          if (Head::callback) {
            Head::callback(Head::value, values);
          }
          return true;
        }
        if (values.size() < Head::nargs.getNargs()) {
          throw Argo::InvalidArgument(
              std::format("Argument {} should take exactly {} value but {}",
                          key, Head::nargs.getNargs(), values.size()));
        }
        if constexpr (std::is_same_v<PArgs, std::tuple<>>) {
          throw Argo::InvalidArgument(
              std::format("Argument {} should take exactly {} value but {}",
                          key, Head::nargs.getNargs(), values.size()));
        } else {
          assignOneArg<Head>(key, values.subspan(0, Head::nargs.getNargs()));
          return PArgAssigner<PArgs>::assign(
              values.subspan(Head::nargs.getNargs()));
        }
      }
    }
    return false;
  }

  template <class Args>
  static auto assignImpl(std::string_view key,
                         std::span<std::string_view> values) {
    [&key, &values]<std::size_t... Is>(std::index_sequence<Is...> /*unused*/) {
      if (!(... ||
            (std::string_view(std::tuple_element_t<Is, Args>::name) == key and
             assignOneArg<std::tuple_element_t<Is, Args>>(key, values)))) {
        throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
      }
    }(std::make_index_sequence<std::tuple_size_v<Args>>());
  }

  template <class T>
  static auto assignFlagImpl(std::string_view key) -> void {
    [&key]<std::size_t... Is>(std::index_sequence<Is...>) {
      if (!(... || [&key]<ArgType Head>() {
            if constexpr (std::derived_from<Head, FlagArgTag>) {
              if (std::string_view(Head::name) == key) {
                Head::value = true;
                return true;
              }
              return false;
            } else {
              return false;
            }
          }.template operator()<std::tuple_element_t<Is, T>>())) {
        throw Argo::InvalidArgument("");
      }
    }(std::make_index_sequence<std::tuple_size_v<T>>());
  }

  static auto assign(std::string_view key,
                     std::span<std::string_view> values) -> void {
    if (key.empty()) {
      if constexpr (!std::is_same_v<PArgs, std::tuple<>>) {
        if (!PArgAssigner<PArgs>::assign(values)) {
          throw InvalidArgument(std::format("Duplicated positional argument"));
        }
        return;
      } else {
        throw Argo::InvalidArgument(
            std::format("Assigner: Invalid argument {}", key));
      }
    }
    assignImpl<Arguments>(key, values);
  };

  static auto assign(std::span<char> key, std::span<std::string_view> values) {
    for (std::size_t i = 0; i < key.size() - 1; i++) {
      assignFlagImpl<Arguments>(GetNameFromShortName<Arguments>::eval(key[i]));
    }
    assignImpl<Arguments>(GetNameFromShortName<Arguments>::eval(key.back()),
                          values);
  };
};

export template <class Args>
auto ValueReset() {
  []<std::size_t... Is>(std::index_sequence<Is...>) {
    (..., []<ArgType T>() {
      if (T::assigned) {
        T::value = typename T::type();
        T::assigned = false;
      }
    }.template operator()<std::tuple_element_t<Is, Args>>());
  }(std::make_index_sequence<std::tuple_size_v<Args>>());
}

};  // namespace Argo

// generator end here
