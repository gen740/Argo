module;

import std_module;
export module Argo:MetaAssigner;
import :Exceptions;
import :Validation;
import :Arg;

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
      throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
    }
  };

  template <int Index, typename Head, typename... Tails>
  struct AssignImpl<Index, std::tuple<Head, Tails...>> {
    template <typename Lhs>
    static auto eval(std::string_view key, std::string_view value) {
      using current = std::remove_cvref_t<decltype(std::get<Index>(std::declval<Lhs>()))>;
      if constexpr (!current::isVariadic) {
        if (std::string_view(std::begin(current::name), std::end(current::name)) == key) {
          if constexpr (std::is_same_v<typename current::type, bool>) {
            if ((value == "true") || (value == "True") || (value == "TRUE") || (value == "1")) {
              current::value = true;
            } else if ((value == "false") || (value == "False") || (value == "FALSE") ||
                       (value == "0")) {
              current::value = false;
            } else {
              throw ParserInternalError("Invalid Argument expect bool");
            }
          } else if constexpr (std::is_integral_v<typename current::type>) {
            typename current::type tmpValue;
            std::from_chars(std::begin(value), std::end(value), tmpValue);
            current::value = tmpValue;
          } else if constexpr (std::is_floating_point_v<typename current::type>) {
            current::value = static_cast<current::type>(std::stod(std::string(value)));
          } else if constexpr (std::is_same_v<typename current::type, const char*>) {
            current::value = value.data();
          } else {
            current::value = static_cast<current::type>(value);
          }
          if constexpr (!std::is_same_v<decltype(current::value), bool>) {
            if (current::validator) {
              (*current::validator)(
                  std::string_view(std::begin(current::name), std::end(current::name)),
                  current::value.value());
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
      throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
    }
  };

  template <int Index, typename Head, typename... Tails>
  struct VariadicAssignImpl<Index, std::tuple<Head, Tails...>> {
    template <typename Lhs>
    static auto eval(std::string_view key, const std::vector<std::string_view>& values) {
      using current = std::remove_cvref_t<decltype(std::get<Index>(std::declval<Lhs>()))>;

      if constexpr (current::isVariadic) {
        if (std::string_view(std::begin(current::name), std::end(current::name)) == key) {
          current::value = std::vector<typename current::baseType>();
          for (auto value : values) {
            if constexpr (std::is_same_v<typename current::baseType, bool>) {
              if ((value == "true") || (value == "True") || (value == "TRUE") || (value == "1")) {
                current::value->push_back(true);
              } else if ((value == "false") || (value == "False") || (value == "FALSE") ||
                         (value == "0")) {
                current::value->push_back(false);
              } else {
                throw ParserInternalError("Invalid Argument expect bool");
              }
            } else if constexpr (std::is_integral_v<typename current::baseType>) {
              typename current::baseType tmpValue;
              std::from_chars(std::begin(value), std::end(value), tmpValue);
              current::value->push_back(tmpValue);
            } else if constexpr (std::is_floating_point_v<typename current::baseType>) {
              current::value->push_back(
                  static_cast<current::baseType>(std::stod(std::string(value))));
            } else if constexpr (std::is_same_v<typename current::baseType, const char*>) {
              current::value->push_back(value.data());
            } else {
              current::value->push_back(static_cast<current::baseType>(value));
            }
            if (current::validator) {
              (*current::validator)(
                  std::string_view(std::begin(current::name), std::end(current::name)),
                  current::value.value());
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
      throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
    }
  };

  template <int Index, typename Head, typename... Tails>
  struct DefaultAssignerImpl<Index, std::tuple<Head, Tails...>> {
    template <typename Lhs>
    static auto eval(std::string_view key) {
      using current = std::remove_cvref_t<decltype(std::get<Index>(std::declval<Lhs>()))>;

      if constexpr (!std::derived_from<current, FlagArgTag>) {
        if (std::string_view(std::begin(current::name), std::end(current::name)) == key) {
          current::value = current::defaultValue;
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
