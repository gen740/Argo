module;

export module Argo:MetaLookup;

import :Exceptions;
import :std_module;
import :Arg;
import :ArgName;

// generator start here

namespace Argo {

using namespace std;

template <class Arguments>
struct GetNameFromShortName {
  template <class Head, class... Tails>
  struct GetNameFromShortNameImpl {
    [[noreturn]] static auto get(char /* unused */) -> string_view {
      throw ParserInternalError("Fail to lookup");
    }
  };

  template <class Head, class... Tails>
  struct GetNameFromShortNameImpl<tuple<Head, Tails...>> {
    static auto get(char key) -> string_view {
      auto name = string_view(Head::name);
      if (Head::name.shortName == key) {
        return name;
      }
      return GetNameFromShortNameImpl<tuple<Tails...>>::get(key);
    }
  };

  static auto eval(char key) -> string_view {
    return GetNameFromShortNameImpl<Arguments>::get(key);
  }
};

/*!
 * Index Search meta function
 */
template <class Tuple, ArgName T, int Index = 0>
struct SearchIndex;

template <ArgName T, size_t Index>
struct SearchIndex<tuple<>, T, Index> {
  static constexpr int value = -1;
};

template <ArgName T, size_t Index, class Head, class... Tails>
struct SearchIndex<tuple<Head, Tails...>, T, Index> {
  static constexpr int value =
      (Head::name == T) ? Index
                        : SearchIndex<tuple<Tails...>, T, Index + 1>::value;
};

/*!
 * Index Search meta function
 */
template <class Tuple, char T, int Index = 0>
struct SearchIndexFromShortName;

template <char T, size_t Index>
struct SearchIndexFromShortName<tuple<>, T, Index> {
  static constexpr int value = -1;
};

template <char T, size_t Index, class Head, class... Tails>
struct SearchIndexFromShortName<tuple<Head, Tails...>, T, Index> {
  static constexpr int value =
      Head::name.shortName == T
          ? Index
          : SearchIndexFromShortName<tuple<Tails...>, T, Index + 1>::value;
};

};  // namespace Argo

// generator end here
