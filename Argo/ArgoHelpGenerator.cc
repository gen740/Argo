module;

#include "Argo/ArgoMacros.hh"

export module Argo:HelpGenerator;

import :Arg;
import :TypeTraits;
import :std_module;

// generator start here

namespace Argo {

using namespace std;

struct ArgInfo {
  string_view name;
  char shortName;
  string_view description;
  bool required;
  string_view typeName;
};

template <class Args>
ARGO_ALWAYS_INLINE constexpr auto HelpGenerator() {
  vector<ArgInfo> ret;
  tuple_type_visit<Args>([&ret]<class T>(T) {
    if constexpr (derived_from<typename T::type, ArgTag>) {
      ret.emplace_back(
          T::type::name.getKey().substr(0, T::type::name.getKeyLen()),  //
          T::type::name.getShortName(),                                 //
          T::type::description,                                         //
          T::type::required,                                            //
          string_view(T::type::typeName));
    } else {
      ret.emplace_back(
          T::type::name.getKey().substr(0, T::type::name.getKeyLen()),  //
          T::type::name.getShortName(),                                 //
          T::type::description,                                         //
          false,                                                        //
          string_view(T::type::typeName));
    }
  });
  return ret;
};

struct SubCommandInfo {
  string_view name;
  string_view description;
};

template <class T>
ARGO_ALWAYS_INLINE constexpr auto SubParserInfo(T subparsers) {
  vector<SubCommandInfo> ret{};
  if constexpr (!is_same_v<T, tuple<>>) {
    apply(
        [&ret]<class... Parser>(Parser... parser) ARGO_ALWAYS_INLINE {
          (..., ret.emplace_back(parser.name.getKey(), parser.description));
        },
        subparsers);
  }
  return ret;
};

}  // namespace Argo

// generator end here
