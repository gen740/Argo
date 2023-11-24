module;

import std_module;

export module Argo:MetaChecker;
import :Exception;

export namespace Argo {

struct CheckOptions {
  bool isBool = false;
  bool isFlag = false;
};

template <class CheckType>
struct TypeChecker {
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
        ret.isBool = std::is_same_v<typename current::type, CheckType>;
        ret.isFlag = current::flagArg;
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

// template <class CheckType>
// struct FlagChecker {
//   template <int Index, typename Head, typename... Tails>
//   struct FlagCheckerImpl {};
//
//   template <int Index>
//   struct FlagCheckerImpl<Index, std::tuple<>> {
//     template <typename Lhs>
//     [[noreturn]] static auto eval(std::string_view key) -> bool {
//       throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
//     }
//   };
//
//   template <int Index, typename Head, typename... Tails>
//   struct FlagCheckerImpl<Index, std::tuple<Head, Tails...>> {
//     template <typename Lhs>
//     static auto eval(std::string_view key) -> bool {
//       using current = std::remove_cvref_t<decltype(std::get<Index>(std::declval<Lhs>()))>;
//       if (std::string_view(std::begin(current::name), std::end(current::name)) == key) {
//         return std::is_same_v<typename current::type, CheckType>;
//       }
//       return FlagCheckerImpl<1 + Index, std::tuple<Tails...>>::template eval<Lhs>(key);
//     }
//   };
//
//   template <typename Lhs>
//   static auto check(std::string_view key) -> bool {
//     return FlagCheckerImpl<0, Lhs>::template eval<Lhs>(key);
//   };
// };

}  // namespace Argo
