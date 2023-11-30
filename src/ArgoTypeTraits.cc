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

template <class T>
struct is_tuple : std::false_type {};

template <class... T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

template <class T>
constexpr bool is_tuple_v = is_tuple<T>::value;

template <class T>
struct is_vector : std::false_type {};

template <class T>
struct is_vector<std::vector<T>> : std::true_type {};

template <class T>
constexpr bool is_vector_v = is_vector<T>::value;

template <class T>
struct vector_base {
  using type = T;
};

template <class T>
struct vector_base<std::vector<T>> {
  using type = T;
};

template <class T>
using vector_base_t = vector_base<T>::type;

template <class T>
struct is_array : std::false_type {};

template <class T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type {};

template <class T>
constexpr bool is_array_v = is_array<T>::value;

template <class T>
struct array_len {
  static constexpr std::size_t value = 0;
};

template <class T, std::size_t N>
struct array_len<std::array<T, N>> {
  static constexpr std::size_t value = N;
};

template <class T>
constexpr std::size_t array_len_v = array_len<T>::value;

template <class T>
struct array_base {
  using type = T;
};

template <class T, std::size_t N>
struct array_base<std::array<T, N>> {
  using type = T;
};

template <class T>
using array_base_t = array_base<T>::type;

};  // namespace Argo
