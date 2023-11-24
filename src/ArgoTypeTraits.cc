module;

import std_module;
export module Argo:TypeTraits;
import :Exceptions;

export namespace Argo {

template <class T>
concept Arithmetic = requires { std::is_arithmetic_v<T>; };

};  // namespace Argo
