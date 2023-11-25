module;

import std_module;

export module Argo:HelpGenerator;
import :MetaLookup;

export namespace Argo {

struct ArgInfo {
  std::string name;
  char shortName;
  std::string_view description;
};

struct HelpGenerator {
  template <int Index, typename Head, typename... Tails>
  struct TypeCheckerImpl {};

  template <int Index>
  struct TypeCheckerImpl<Index, std::tuple<>> {
    template <typename Lhs>
    static auto eval(std::vector<ArgInfo>& /* unused */) {}
  };

  template <int Index, typename Head, typename... Tails>
  struct TypeCheckerImpl<Index, std::tuple<Head, Tails...>> {
    template <typename Lhs>
    static auto eval(std::vector<ArgInfo>& ret) {
      ret.emplace_back(ArrayToString(Head::name), Head::shortName, Head::description);
      TypeCheckerImpl<1 + Index, std::tuple<Tails...>>::template eval<Lhs>(ret);
    }
  };

  template <typename Lhs>
  static auto generate() {
    auto ret = std::vector<ArgInfo>();
    TypeCheckerImpl<0, Lhs>::template eval<Lhs>(ret);
    return ret;
  };
};

}  // namespace Argo
