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

template <typename Arguments, typename PositionalArgument>
struct Assigner {
  template <int Index, typename Head, typename... Tails>
  struct AssignImpl {};

  template <int Index>
  struct AssignImpl<Index, std::tuple<>> {
    __attribute__((always_inline)) static auto eval(std::string_view key,
                                                    std::span<std::string_view>& /* unused */) {
      throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
    }
  };

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
      return true;
    } else {
      if constexpr (Head::nargs.getNargsChar() == '?') {
        if (values.empty()) {
          Head::value = Head::defaultValue;
          Head::assigned = true;
          return true;
        }
        if (values.size() == 1) {
          Head::value = caster<typename Head::baseType>(values[0]);
          Head::assigned = true;
          if (Head::validator) {
            (*Head::validator)(key, Head::value);
          }
          if (Head::callback) {
            Head::callback(key, Head::value);
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
          Head::value.push_back(caster<typename Head::baseType>(value));
        }
        Head::assigned = true;
        if (Head::validator) {
          (*Head::validator)(key, Head::value);
        }
        if (Head::callback) {
          Head::callback(key, Head::value);
        }
        return true;
      } else if constexpr (Head::nargs.getNargsChar() == '+') {
        if (values.empty()) {
          throw Argo::InvalidArgument(
              std::format("Argument {} should take more than one value", key));
        }
        for (const auto& value : values) {
          Head::value.push_back(caster<typename Head::baseType>(value));
        }
        Head::assigned = true;
        if (Head::validator) {
          (*Head::validator)(key, Head::value);
        }
        if (Head::callback) {
          Head::callback(key, Head::value);
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
          (*Head::validator)(key, Head::value);
        }
        if (Head::callback) {
          Head::callback(key, Head::value);
        }
        return true;
      } else {
        if (values.size() == Head::nargs.getNargs()) {
          Head::value = typename Head::type();
          for (const auto& j : values) {
            Head::value.push_back(caster<typename Head::baseType>(j));
          }
          Head::assigned = true;
          if (Head::validator) {
            (*Head::validator)(key, Head::value);
          }
          if (Head::callback) {
            Head::callback(key, Head::value);
          }
          return true;
        }
        if (values.size() < Head::nargs.getNargs()) {
          throw Argo::InvalidArgument(std::format("Argument {} should take exactly {} value but {}",
                                                  key, Head::nargs.getNargs(), values.size()));
        } else {
          if constexpr (std::is_same_v<PositionalArgument, void>) {
            throw Argo::InvalidArgument(std::format(
                "Argument {} should take exactly one value but {}", key, values.size()));
          } else {
            if (PositionalArgument::assigned) {
              throw Argo::InvalidArgument(std::format(
                  "Argument {} should take exactly one value but {}", key, values.size()));
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

  template <int Index, ArgType Head, typename... Tails>
  struct AssignImpl<Index, std::tuple<Head, Tails...>> {
    __attribute__((always_inline)) static auto eval(std::string_view key,
                                                    std::span<std::string_view> values) {
      if (std::string_view(Head::name) == key) {
        if (assignOneArg<Head>(key, std::span<std::string_view>(values.begin(), values.end()))) {
          return;
        }
      }
      AssignImpl<1 + Index, std::tuple<Tails...>>::eval(key, values);
    }
  };

  template <int Index, typename Head, typename... Tails>
  struct AssignFlagImpl {};

  template <int Index>
  struct AssignFlagImpl<Index, std::tuple<>> {
    static auto eval(std::string_view key) {
      throw Argo::InvalidArgument(std::format("Assigner: Invalid argument {}", key));
    }
  };

  template <int Index, ArgType Head, typename... Tails>
  struct AssignFlagImpl<Index, std::tuple<Head, Tails...>> {
    static auto eval(std::string_view key) {
      if constexpr (std::derived_from<Head, FlagArgTag>) {
        if (std::string_view(Head::name) == key) {
          Head::value = true;
          return;
        }
      }
      AssignFlagImpl<1 + Index, std::tuple<Tails...>>::eval(key);
    }
  };

  static auto assign(std::string_view key, std::span<std::string_view> values) {
    if (key.empty()) {
      if constexpr (!std::is_same_v<PositionalArgument, void>) {
        if (PositionalArgument::assigned) {
          throw InvalidArgument(std::format("Duplicated positional argument"));
        }
        assignOneArg<PositionalArgument>(std::string_view(PositionalArgument::name), values);
        return;
      } else {
        throw InvalidArgument(std::format("No keys specified"));
      }
    }
    AssignImpl<0, Arguments>::eval(key, values);
  };

  static auto assign(std::span<char> key, std::span<std::string_view> values) {
    std::string name;
    for (std::size_t i = 0; i < key.size() - 1; i++) {
      name = GetNameFromShortName<Arguments>::eval(key[i]);
      AssignFlagImpl<0, Arguments>::eval(name);
    }
    name = GetNameFromShortName<Arguments>::eval(key.back());
    AssignImpl<0, Arguments>::eval(name, values);
  };
};

};  // namespace Argo
