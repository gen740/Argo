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

};  // namespace Argo
