module;

export module Argo:MetaAssigner;

import :Exceptions;
import :Validation;
import :Arg;
import :std_module;

export namespace Argo {

/*!
 * Helper class of assigning value
 */
struct Assigner {
  template <int Index, typename Head, typename... Tails>
  struct AssignImpl {};

  template <int Index>
  struct AssignImpl<Index, std::tuple<>> {
    template <typename Lhs>
    static auto eval(std::string_view key, [[maybe_unused]] std::string_view val) {
      throw Argo::InvalidArgument(std::format("Assigner: Invalid argument {}", key));
    }
  };

  template <int Index, typename Head, typename... Tails>
  struct AssignImpl<Index, std::tuple<Head, Tails...>> {
    template <typename Lhs>
    static auto eval(std::string_view key, std::string_view value) {
      if constexpr (!Head::isVariadic) {
        if (std::string_view(std::begin(Head::name), std::end(Head::name)) == key) {
          if constexpr (std::is_same_v<typename Head::type, bool>) {
            if ((value == "true") || (value == "True") || (value == "TRUE") || (value == "1")) {
              Head::value = true;
            } else if ((value == "false") || (value == "False") || (value == "FALSE") ||
                       (value == "0")) {
              Head::value = false;
            } else {
              throw ParserInternalError("Invalid Argument expect bool");
            }
          } else if constexpr (std::is_integral_v<typename Head::type>) {
            typename Head::type tmpValue;
            std::from_chars(std::begin(value), std::end(value), tmpValue);
            Head::value = tmpValue;
          } else if constexpr (std::is_floating_point_v<typename Head::type>) {
            Head::value = static_cast<Head::type>(std::stod(std::string(value)));
          } else if constexpr (std::is_same_v<typename Head::type, const char*>) {
            Head::value = value.data();
          } else {
            Head::value = static_cast<Head::type>(value);
          }
          if constexpr (!std::is_same_v<decltype(Head::value), bool>) {
            if (Head::validator) {
              (*Head::validator)(std::string_view(std::begin(Head::name), std::end(Head::name)),
                                 Head::value.value());
            }
          }
          return;
        }
      }
      AssignImpl<1 + Index, std::tuple<Tails...>>::template eval<Lhs>(key, value);
    }
  };

  template <typename Lhs>
  static auto assign(std::string_view key, std::string_view val) {
    AssignImpl<0, Lhs>::template eval<Lhs>(key, val);
  };
};

/*!
 * Helper class of assigning value
 */
struct VariadicAssigner {
  template <int Index, typename Head, typename... Tails>
  struct VariadicAssignImpl {};

  template <int Index>
  struct VariadicAssignImpl<Index, std::tuple<>> {
    template <typename Lhs>
    static auto eval(std::string_view key,
                     [[maybe_unused]] const std::vector<std::string_view>& values) {
      throw Argo::InvalidArgument(std::format("Variadic: Invalid argument {}", key));
    }
  };

  template <int Index, typename Head, typename... Tails>
  struct VariadicAssignImpl<Index, std::tuple<Head, Tails...>> {
    template <typename Lhs>
    static auto eval(std::string_view key, const std::vector<std::string_view>& values) {
      if constexpr (Head::isVariadic) {
        if (std::string_view(std::begin(Head::name), std::end(Head::name)) == key) {
          Head::value = std::vector<typename Head::baseType>();
          for (auto value : values) {
            if constexpr (std::is_same_v<typename Head::baseType, bool>) {
              if ((value == "true") || (value == "True") || (value == "TRUE") || (value == "1")) {
                Head::value->push_back(true);
              } else if ((value == "false") || (value == "False") || (value == "FALSE") ||
                         (value == "0")) {
                Head::value->push_back(false);
              } else {
                throw ParserInternalError("Invalid Argument expect bool");
              }
            } else if constexpr (std::is_integral_v<typename Head::baseType>) {
              typename Head::baseType tmpValue;
              std::from_chars(std::begin(value), std::end(value), tmpValue);
              Head::value->push_back(tmpValue);
            } else if constexpr (std::is_floating_point_v<typename Head::baseType>) {
              Head::value->push_back(static_cast<Head::baseType>(std::stod(std::string(value))));
            } else if constexpr (std::is_same_v<typename Head::baseType, const char*>) {
              Head::value->push_back(value.data());
            } else {
              Head::value->push_back(static_cast<Head::baseType>(value));
            }
            if (Head::validator) {
              (*Head::validator)(std::string_view(std::begin(Head::name), std::end(Head::name)),
                                 Head::value.value());
            }
          }
          return;
        }
      }

      VariadicAssignImpl<1 + Index, std::tuple<Tails...>>::template eval<Lhs>(key, values);
    }
  };

  template <typename Lhs>
  static auto assign(std::string_view key, const std::vector<std::string_view>& val) {
    VariadicAssignImpl<0, Lhs>::template eval<Lhs>(key, val);
  };
};

struct DefaultAssigner {
  template <int Index, typename Head, typename... Tails>
  struct DefaultAssignerImpl {};

  template <int Index>
  struct DefaultAssignerImpl<Index, std::tuple<>> {
    template <typename Lhs>
    static auto eval(std::string_view key) {
      throw Argo::InvalidArgument(std::format("Default: Invalid argument {}", key));
    }
  };

  template <int Index, typename Head, typename... Tails>
  struct DefaultAssignerImpl<Index, std::tuple<Head, Tails...>> {
    template <typename Lhs>
    static auto eval(std::string_view key) {
      if constexpr (!std::derived_from<Head, FlagArgTag>) {
        if (std::string_view(std::begin(Head::name), std::end(Head::name)) == key) {
          Head::value = Head::defaultValue;
          return;
        }
      }

      DefaultAssignerImpl<1 + Index, std::tuple<Tails...>>::template eval<Lhs>(key);
    }
  };

  template <typename Lhs>
  static auto assign(std::string_view key) {
    DefaultAssignerImpl<0, Lhs>::template eval<Lhs>(key);
  };
};

};  // namespace Argo
