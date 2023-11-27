module;

export module Argo:MetaLookup;

import :Exceptions;
import :std_module;

export namespace Argo {

template <std::size_t N>
constexpr auto ArrayToString(const std::array<char, N>& from) -> std::string {
  std::string ret;
  for (std::size_t i = 0; i < N; i++) {
    ret.push_back(from[i]);
  }
  return ret;
}

template <class Arguments>
struct GetNameFromShortName {
  template <class Head, class... Tails>
  struct GetNameFromShortNameImpl {
    [[noreturn]] static auto get([[maybe_unused]] char key) -> std::string {
      throw ParserInternalError("Fail to lookup");
    }
  };

  template <class Head, class... Tails>
  struct GetNameFromShortNameImpl<std::tuple<Head, Tails...>> {
    static auto get(char key) -> std::string {
      auto name = ArrayToString(Head::name);
      if (Head::shortName == key) {
        return name;
      }
      return GetNameFromShortNameImpl<std::tuple<Tails...>>::get(key);
    }
  };

  static auto eval(char key) -> std::string {
    return GetNameFromShortNameImpl<Arguments>::get(key);
  }
};

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

/*!
 * Index Search meta function
 */
template <typename Tuple, char T, int Index = 0>
struct SearchIndexFromShortName;

template <char T, std::size_t Index>
struct SearchIndexFromShortName<std::tuple<>, T, Index> {
  static constexpr int value = -1;
};

template <char T, std::size_t Index, typename Head, typename... Tails>
struct SearchIndexFromShortName<std::tuple<Head, Tails...>, T, Index> {
  static constexpr int value =
      Head::shortName == T ? Index
                           : SearchIndexFromShortName<std::tuple<Tails...>, T, Index + 1>::value;
};

};  // namespace Argo
