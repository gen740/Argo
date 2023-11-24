module;

import std_module;
export module Argo:MetaAssigner;
import :Exceptions;
import :Validation;

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
          std::from_chars(std::begin(value), std::end(value), current::value);
        } else if constexpr (std::is_floating_point_v<typename current::type>) {
          current::value = static_cast<current::type>(std::stod(std::string(value)));
        } else if constexpr (std::is_same_v<typename current::type, const char*>) {
          current::value = value.data();
        } else {
          current::value = static_cast<current::type>(value);
        }
        if (current::validator) {
          (*current::validator)(
              std::string_view(std::begin(current::name), std::end(current::name)), current::value);
        }
        return;
      }
      AssignImpl<1 + Index, std::tuple<Tails...>>::template eval<Lhs>(key, value);
    }
  };

  template <typename Lhs>
  static auto assign(std::string_view key, std::string_view val) {
    AssignImpl<0, Lhs>::template eval<Lhs>(key, val);
  };
};

};  // namespace Argo
