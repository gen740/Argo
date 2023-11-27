module;

export module Argo:MetaAssigner;

import :Exceptions;
import :Validation;
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
    throw ParserInternalError("Invalid Argument expect bool");
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

struct Assigner {
  template <int Index, typename Head, typename... Tails>
  struct AssignImpl {};

  template <int Index>
  struct AssignImpl<Index, std::tuple<>> {
    template <typename Lhs>
    __attribute__((always_inline)) static auto eval(
        std::string_view key, const std::vector<std::string_view>& /* unused */) {
      throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
    }
  };

  template <int Index, ArgType Head, typename... Tails>
  struct AssignImpl<Index, std::tuple<Head, Tails...>> {
    template <typename Lhs>

    __attribute__((always_inline)) static auto eval(std::string_view key,
                                                    const std::vector<std::string_view>& values) {
      if (std::string_view(Head::name) == key) {
        if constexpr (std::derived_from<Head, FlagArgTag>) {
          if (!values.empty()) {
            throw Argo::InvalidArgument(std::format("Flag {} can not take value", key));
          }
          Head::value = true;
          return;
        } else {
          if constexpr (Head::nargs.getNargsChar() == '?') {
            if (values.empty()) {
              Head::value = Head::defaultValue;
              return;
            }
            if (values.size() == 1) {
              Head::value = caster<typename Head::baseType>(values[0]);
              if (Head::validator) {
                (*Head::validator)(key, Head::value.value());
              }
              return;
            }
            throw Argo::InvalidArgument(std::format(
                "Argument {} cannot take more than one value got {}", key, values.size()));
          } else if constexpr (Head::nargs.getNargsChar() == '*') {
            if (values.empty()) {
              Head::value = Head::defaultValue;
              return;
            }
            Head::value = typename Head::type();
            for (const auto& value : values) {
              Head::value->push_back(caster<typename Head::baseType>(value));
            }
            if (Head::validator) {
              (*Head::validator)(key, Head::value.value());
            }
            return;
          } else if constexpr (Head::nargs.getNargsChar() == '+') {
            if (values.empty()) {
              throw Argo::InvalidArgument(
                  std::format("Argument {} should take more than one value", key));
            }
            Head::value = typename Head::type();
            for (const auto& value : values) {
              Head::value->push_back(caster<typename Head::baseType>(value));
            }
            if (Head::validator) {
              (*Head::validator)(key, Head::value.value());
            }
            return;
          } else if constexpr (Head::nargs.getNargs() == 1) {
            if (values.empty()) {
              throw Argo::InvalidArgument(
                  std::format("Argument {} should take exactly one value but zero", key));
            }
            if (values.size() > 1) {
              throw Argo::InvalidArgument(std::format(
                  "Argument {} should take exactly one value but {}", key, values.size()));
            }
            Head::value = caster<typename Head::baseType>(values[0]);
            if (Head::validator) {
              (*Head::validator)(key, Head::value.value());
            }
            return;
          } else {
            if (values.size() != Head::nargs.getNargs()) {
              throw Argo::InvalidArgument(
                  std::format("Argument {} should take exactly {} value but {}", key,
                              Head::nargs.getNargs(), values.size()));
            }
            Head::value = typename Head::type();
            for (const auto& j : values) {
              Head::value->push_back(caster<typename Head::baseType>(j));
            }
            if (Head::validator) {
              (*Head::validator)(key, Head::value.value());
            }
            return;
          }
        }
      }
      AssignImpl<1 + Index, std::tuple<Tails...>>::template eval<Lhs>(key, values);
    }
  };

  template <int Index, typename Head, typename... Tails>
  struct AssignFlagImpl {};

  template <int Index>
  struct AssignFlagImpl<Index, std::tuple<>> {
    template <typename Lhs>
    __attribute__((always_inline)) static auto eval(std::string_view key) {
      throw Argo::InvalidArgument(std::format("Assigner: Invalid argument {}", key));
    }
  };

  template <int Index, ArgType Head, typename... Tails>
  struct AssignFlagImpl<Index, std::tuple<Head, Tails...>> {
    template <typename Lhs>

    __attribute__((always_inline)) static auto eval(std::string_view key) {
      if constexpr (std::derived_from<Head, FlagArgTag>) {
        if (std::string_view(Head::name) == key) {
          Head::value = true;
          return;
        }
      }
      AssignFlagImpl<1 + Index, std::tuple<Tails...>>::template eval<Lhs>(key);
    }
  };

  template <typename Lhs>
  static auto assign(std::string_view key, const std::vector<std::string_view>& values) {
    AssignImpl<0, Lhs>::template eval<Lhs>(key, values);
  };

  template <typename Lhs>
  static auto assign(const std::vector<char>& key, const std::vector<std::string_view>& values) {
    std::string name;
    for (std::size_t i = 0; i < key.size() - 1; i++) {
      name = GetNameFromShortName<Lhs>::eval(key[i]);
      AssignFlagImpl<0, Lhs>::template eval<Lhs>(name);
    }
    name = GetNameFromShortName<Lhs>::eval(key.back());
    AssignImpl<0, Lhs>::template eval<Lhs>(name, values);
  };
};

};  // namespace Argo
