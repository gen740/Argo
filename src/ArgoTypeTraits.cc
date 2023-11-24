module;

import std_module;
export module Argo:TypeTraits;
import :Exceptions;

export namespace Argo {

template <auto T, auto U>
struct ConstexprStringCmp {
  static consteval auto eval() {
    if (T.size() != U.size()) {
      return false;
    }
    for (int i = 0; i < T.size(); i++) {
      if (T[i] != U[i]) {
        return false;
      }
    }
    return true;
  }
};

/*!
 * Index Search meta function
 */
template <typename Tuple, auto T, int Index = 0>
struct SearchIndex;

template <auto T, std::size_t Index>
struct SearchIndex<std::tuple<>, T, Index> {
  static constexpr int value = -1;
};

template <auto T, std::size_t Index, typename Head, typename... Tails>
struct SearchIndex<std::tuple<Head, Tails...>, T, Index> {
  static constexpr int value = ConstexprStringCmp<Head::name, T>::eval()
                                   ? Index
                                   : SearchIndex<std::tuple<Tails...>, T, Index + 1>::value;
};

};  // namespace Argo
