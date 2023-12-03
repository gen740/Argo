module;

export module Argo:MetaLookup;

import :Exceptions;
import :std_module;
import :Arg;

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
      if (Head::name.shortName == key) {
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
template <class Tuple, ArgName T, int Index = 0>
struct SearchIndex;

template <ArgName T, std::size_t Index>
struct SearchIndex<std::tuple<>, T, Index> {
  static constexpr int value = -1;
};

template <ArgName T, std::size_t Index, class Head, class... Tails>
struct SearchIndex<std::tuple<Head, Tails...>, T, Index> {
  static constexpr int value =
      (Head::name == T)
          ? Index
          : SearchIndex<std::tuple<Tails...>, T, Index + 1>::value;
};

/*!
 * Index Search meta function
 */
template <class Tuple, char T, int Index = 0>
struct SearchIndexFromShortName;

template <char T, std::size_t Index>
struct SearchIndexFromShortName<std::tuple<>, T, Index> {
  static constexpr int value = -1;
};

template <char T, std::size_t Index, class Head, class... Tails>
struct SearchIndexFromShortName<std::tuple<Head, Tails...>, T, Index> {
  static constexpr int value =
      Head::name.shortName == T
          ? Index
          : SearchIndexFromShortName<std::tuple<Tails...>, T, Index + 1>::value;
};

};  // namespace Argo
