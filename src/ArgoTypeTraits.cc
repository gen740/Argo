module;

export module Argo:TypeTraits;
import :Exceptions;
import :std_module;

export namespace Argo {

template <auto Value>
struct IdentityHolder {
  static constexpr auto value = Value;
};

template <class T>
concept Arithmetic = requires { std::is_arithmetic_v<T>; };

template <typename T>
struct is_tuple : std::false_type {};

template <typename... Types>
struct is_tuple<std::tuple<Types...>> : std::true_type {};

template <typename T>
constexpr bool is_tuple_v = is_tuple<T>::value;

};  // namespace Argo
