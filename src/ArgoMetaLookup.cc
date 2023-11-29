module;

export module Argo:MetaLookup;

import :Exceptions;
import :std_module;

export namespace Argo {

template <class Arguments>
struct GetNameFromShortName {
  template <class Head, class... Tails>
  struct GetNameFromShortNameImpl {
    [[noreturn]] static auto get(char /* unused */) -> std::string_view {
      throw ParserInternalError("Fail to lookup");
    }
  };

  template <class Head, class... Tails>
  struct GetNameFromShortNameImpl<std::tuple<Head, Tails...>> {
    static auto get(char key) -> std::string_view {
      auto name = std::string_view(Head::name);
      if (Head::shortName == key) {
        return name;
      }
      return GetNameFromShortNameImpl<std::tuple<Tails...>>::get(key);
    }
  };

  static auto eval(char key) -> std::string_view {
    return GetNameFromShortNameImpl<Arguments>::get(key);
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
  static constexpr int value =
      (Head::name == T) ? Index : SearchIndex<std::tuple<Tails...>, T, Index + 1>::value;
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
