module;

import std_module;

export module Argo:MetaLookup;
import :Exceptions;

export namespace Argo {

template <std::size_t N>
constexpr auto ArrayToString(std::array<char, N> from) -> std::string {
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

};  // namespace Argo
