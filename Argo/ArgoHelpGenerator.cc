module;

#include "Argo/ArgoMacros.hh"

export module Argo:HelpGenerator;

import :Arg;
import :TypeTraits;
import :std_module;

// generator start here

namespace Argo {

struct ArgInfo {
  std::string_view name;
  char shortName;
  std::string_view description;
  bool required;
  std::string_view typeName;
};

template <class Args>
ARGO_ALWAYS_INLINE constexpr auto HelpGenerator() {
  std::vector<ArgInfo> ret;
  tuple_type_visit<Args>([&ret]<class T>(T) {
    if constexpr (std::derived_from<typename T::type, ArgTag>) {
      ret.emplace_back(
          T::type::name.getKey().substr(0, T::type::name.getKeyLen()),  //
          T::type::name.getShortName(),                                 //
          T::type::description,                                         //
          T::type::required,                                            //
          std::string_view(T::type::typeName));
    } else {
      ret.emplace_back(
          T::type::name.getKey().substr(0, T::type::name.getKeyLen()),  //
          T::type::name.getShortName(),                                 //
          T::type::description,                                         //
          false,                                                        //
          std::string_view(T::type::typeName));
    }
  });
  return ret;
};

struct SubCommandInfo {
  std::string_view name;
  std::string_view description;
};

template <class T>
ARGO_ALWAYS_INLINE constexpr auto SubParserInfo(T subparsers) {
  std::vector<SubCommandInfo> ret{};
  if constexpr (!std::is_same_v<T, std::tuple<>>) {
    std::apply(
        [&ret]<class... Parser>(Parser... parser) ARGO_ALWAYS_INLINE {
          (..., ret.emplace_back(parser.name.getKey(), parser.description));
        },
        subparsers);
  }
  return ret;
};

}  // namespace Argo

// generator end here
