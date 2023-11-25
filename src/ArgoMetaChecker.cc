module;

import std_module;

export module Argo:MetaChecker;

import :Exceptions;
import :Initializer;
import :NArgs;
import :Arg;

export namespace Argo {

struct CheckOptions {
  bool isFlag = false;
  NArgs nargs = NArgs('?');
};

struct Checker {
  template <int Index, typename Head, typename... Tails>
  struct TypeCheckerImpl {};

  template <int Index>
  struct TypeCheckerImpl<Index, std::tuple<>> {
    template <typename Lhs>
    [[noreturn]] static auto eval(std::string_view key) -> CheckOptions {
      throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
    }
  };

  template <int Index, typename Head, typename... Tails>
  struct TypeCheckerImpl<Index, std::tuple<Head, Tails...>> {
    template <typename Lhs>
    static auto eval(std::string_view key) -> CheckOptions {
      using current = std::remove_cvref_t<decltype(std::get<Index>(std::declval<Lhs>()))>;
      if (std::string_view(std::begin(current::name), std::end(current::name)) == key) {
        auto ret = CheckOptions();
        if constexpr (std::derived_from<current, FlagArgTag>) {
          ret.nargs.nargs = 0;
          ret.nargs.nargs_char = NULLCHAR;
          ret.isFlag = true;
        } else {
          ret.nargs = current::nargs;
        }
        return ret;
      }
      return TypeCheckerImpl<1 + Index, std::tuple<Tails...>>::template eval<Lhs>(key);
    }
  };

  template <typename Lhs>
  static auto check(std::string_view key) -> CheckOptions {
    return TypeCheckerImpl<0, Lhs>::template eval<Lhs>(key);
  };
};

}  // namespace Argo
