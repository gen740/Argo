module;

export module Argo:TypeTraits;
import :Exceptions;
import :std_module;

// generator start here

export namespace Argo {

using namespace std;

template <class T>
struct is_tuple : false_type {};

template <class... T>
struct is_tuple<tuple<T...>> : true_type {};

template <class T>
constexpr bool is_tuple_v = is_tuple<T>::value;

template <class T>
struct is_vector : false_type {};

template <class T>
struct is_vector<vector<T>> : true_type {};

template <class T>
constexpr bool is_vector_v = is_vector<T>::value;

template <class T>
struct vector_base {
  using type = T;
};

template <class T>
struct vector_base<vector<T>> {
  using type = T;
};

template <class T>
using vector_base_t = vector_base<T>::type;

template <class T>
struct is_array : false_type {};

template <class T, size_t N>
struct is_array<array<T, N>> : true_type {};

template <class T>
constexpr bool is_array_v = is_array<T>::value;

template <class T>
struct array_len {
  static constexpr size_t value = 0;
};

template <class T, size_t N>
struct array_len<array<T, N>> {
  static constexpr size_t value = N;
};

template <class T>
constexpr size_t array_len_v = array_len<T>::value;

template <class T>
struct array_base {
  using type = T;
};

template <class T, size_t N>
struct array_base<array<T, N>> {
  using type = T;
};

template <class T>
using array_base_t = array_base<T>::type;

template <class T, class... U>
struct tuple_append {
  using type = tuple<U..., T>;
};

template <class T, class... U>
struct tuple_append<tuple<U...>, T> {
  using type = tuple<U..., T>;
};

template <class... T>
using tuple_append_t = typename tuple_append<T...>::type;

};  // namespace Argo

// generator end here
