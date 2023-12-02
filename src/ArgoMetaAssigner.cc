module;

export module Argo:MetaAssigner;

import :Exceptions;
import :Validation;
import :TypeTraits;
import :MetaLookup;
import :Arg;
import :std_module;

export namespace Argo {

/*!
 * Helper class of assigning value
 */
template <class Type>
constexpr auto caster(std::string_view value) -> Type {
  if constexpr (std::is_same_v<Type, bool>) {
    if ((value == "true") || (value == "True") || (value == "TRUE") || (value == "1")) {
      return true;
    }
    if ((value == "false") || (value == "False") || (value == "FALSE") || (value == "0")) {
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
  ((std::get<N>(t) = caster<std::remove_cvref_t<decltype(std::get<N>(t))>>(v[N])), ...);
}

template <class Arguments, class PositionalArgument>
struct Assigner {
  template <ArgType Head>
  static constexpr auto assignOneArg(std::string_view key,
                                     std::span<std::string_view> values) -> bool {
    if constexpr (std::derived_from<Head, FlagArgTag>) {
      if (!values.empty()) {
        if constexpr (std::is_same_v<PositionalArgument, void>) {
          throw Argo::InvalidArgument(std::format("Flag {} can not take value", key));
        } else {
          if (PositionalArgument::assigned) {
            throw Argo::InvalidArgument(std::format("Flag {} can not take value", key));
          }
          Head::value = true;
          Head::assigned = true;
          assignOneArg<PositionalArgument>(std::string_view(PositionalArgument::name), values);
          return true;
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
        if constexpr (std::is_same_v<PositionalArgument, void>) {
          throw Argo::InvalidArgument(std::format(
              "Argument {} cannot take more than one value got {}", key, values.size()));
        } else {
          if (PositionalArgument::assigned) {
            throw Argo::InvalidArgument(std::format(
                "Argument {} cannot take more than one value got {}", key, values.size()));
          }
          assignOneArg<Head>(key, values.subspan(0, 1));
          assignOneArg<PositionalArgument>(std::string_view(PositionalArgument::name),
                                           values.subspan(1));
          return true;
        }
      } else if constexpr (Head::nargs.getNargsChar() == '*') {
        if (values.empty()) {
          Head::value = Head::defaultValue;
          Head::assigned = true;
          return true;
        }
        for (const auto& value : values) {
          Head::value.emplace_back(caster<vector_base_t<typename Head::type>>(value));
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
          throw Argo::InvalidArgument(
              std::format("Argument {} should take exactly one value but zero", key));
        }
        if (values.size() > 1) {
          if constexpr (std::is_same_v<PositionalArgument, void>) {
            throw Argo::InvalidArgument(std::format(
                "Argument {} should take exactly one value but {}", key, values.size()));
          } else {
            if (PositionalArgument::assigned) {
              throw Argo::InvalidArgument(std::format(
                  "Argument {} should take exactly one value but {}", key, values.size()));
            }
            assignOneArg<Head>(key, values.subspan(0, 1));
            assignOneArg<PositionalArgument>(std::string_view(PositionalArgument::name),
                                             values.subspan(1));
            return true;
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
              Head::value[idx] = caster<array_base_t<typename Head::type>>(values[idx]);
            }
          } else if constexpr (is_tuple_v<typename Head::type>) {
            tupleAssign(Head::value, values,
                        std::make_index_sequence<std::tuple_size_v<typename Head::type>>());
          } else {
            for (const auto& value : values) {
              Head::value.emplace_back(caster<vector_base_t<typename Head::type>>(value));
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
          throw Argo::InvalidArgument(std::format("Argument {} should take exactly {} value but {}",
                                                  key, Head::nargs.getNargs(), values.size()));
        } else {
          if constexpr (std::is_same_v<PositionalArgument, void>) {
            throw Argo::InvalidArgument(
                std::format("Argument {} should take exactly {} value but {}", key,
                            Head::nargs.getNargs(), values.size()));
          } else {
            if (PositionalArgument::assigned) {
              throw Argo::InvalidArgument(
                  std::format("Argument {} should take exactly {} value but {}", key,
                              Head::nargs.getNargs(), values.size()));
            }
            assignOneArg<Head>(key, values.subspan(0, Head::nargs.getNargs()));
            assignOneArg<PositionalArgument>(std::string_view(PositionalArgument::name),
                                             values.subspan(Head::nargs.getNargs()));
            return true;
          }
        }
      }
    }
    return false;
  }

  template <class Args>
  static auto assignImpl(std::string_view key, std::span<std::string_view> values) {
    [&key, &values]<std::size_t... Is>(std::index_sequence<Is...> /*unused*/) {
      if (!(... || (std::string_view(std::tuple_element_t<Is, Args>::name) == key and
                    assignOneArg<std::tuple_element_t<Is, Args>>(key, values)))) {
        throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
      }
    }(std::make_index_sequence<std::tuple_size_v<Args>>());
  }

  template <class T>
  static auto assignFlagImpl(std::string_view key) {
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

  static auto assign(std::string_view key, std::span<std::string_view> values) {
    if (key.empty()) {
      if constexpr (!std::is_same_v<PositionalArgument, void>) {
        if (PositionalArgument::assigned) {
          throw InvalidArgument(std::format("Duplicated positional argument"));
        }
        assignOneArg<PositionalArgument>(std::string_view(PositionalArgument::name), values);
        return;
      } else {
        throw Argo::InvalidArgument(std::format("Assigner: Invalid argument {}", key));
      }
    }
    assignImpl<Arguments>(key, values);
  };

  static auto assign(std::span<char> key, std::span<std::string_view> values) {
    for (std::size_t i = 0; i < key.size() - 1; i++) {
      assignFlagImpl<Arguments>(GetNameFromShortName<Arguments>::eval(key[i]));
    }
    assignImpl<Arguments>(GetNameFromShortName<Arguments>::eval(key.back()), values);
  };
};

template <class Args>
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
