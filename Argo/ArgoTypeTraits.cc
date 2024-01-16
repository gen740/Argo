module;

export module Argo:TypeTraits;
import :Exceptions;
import :std_module;

#include "Argo/ArgoMacros.hh"

// generator start here

export namespace Argo {

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

template <class T, size_t N>
struct is_array<std::array<T, N>> : std::true_type {};

template <class T>
constexpr bool is_array_v = is_array<T>::value;

template <class T>
struct array_len {
  static constexpr size_t value = 0;
};

template <class T, size_t N>
struct array_len<std::array<T, N>> {
  static constexpr size_t value = N;
};

template <class T>
constexpr size_t array_len_v = array_len<T>::value;

template <class T>
struct array_base {
  using type = T;
};

template <class T, size_t N>
struct array_base<std::array<T, N>> {
  using type = T;
};

template <class T>
using array_base_t = array_base<T>::type;

template <class T, class... U>
struct tuple_append {
  using type = std::tuple<U..., T>;
};

template <class T, class... U>
struct tuple_append<std::tuple<U...>, T> {
  using type = std::tuple<U..., T>;
};

template <class... T>
using tuple_append_t = typename tuple_append<T...>::type;

template <class... T>
struct type_sequence {};

template <class T>
struct make_type_sequence {};

template <class... T>
struct make_type_sequence<std::tuple<T...>> {
  using type = type_sequence<T...>;
};

template <class T>
using make_type_sequence_t = make_type_sequence<T>::type;

template <class Tuple, class T>
ARGO_ALWAYS_INLINE constexpr auto tuple_type_visit(T fun) {
  [&fun]<class... U>(type_sequence<U...>) ARGO_ALWAYS_INLINE {
    (fun(std::type_identity<U>()), ...);
  }(make_type_sequence_t<Tuple>());
}

template <class Tuple, class T>
ARGO_ALWAYS_INLINE constexpr auto tuple_type_or_visit(T fun) -> bool {
  return [&fun]<class... U>(type_sequence<U...>) ARGO_ALWAYS_INLINE {
    return (fun(std::type_identity<U>()) || ...);
  }(make_type_sequence_t<Tuple>());
}

template <class Tuple, class T>
ARGO_ALWAYS_INLINE constexpr auto tuple_type_and_visit(T fun) -> bool {
  return [&fun]<class... U>(type_sequence<U...>) ARGO_ALWAYS_INLINE {
    return (fun(std::type_identity<U>()) && ...);
  }(make_type_sequence_t<Tuple>());
}

};  // namespace Argo

// generator end here
