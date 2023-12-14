module;

#include "Argo/ArgoMacros.hh"

export module Argo:HelpGenerator;
import :Arg;
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
struct HelpGenerator {};

template <class... Args>
struct HelpGenerator<tuple<Args...>> {
  ARGO_ALWAYS_INLINE constexpr static auto generate() -> vector<ArgInfo> {
    vector<ArgInfo> ret;
    (
        [&ret]() ARGO_ALWAYS_INLINE {
          if constexpr (derived_from<Args, ArgTag>) {
            ret.emplace_back(
                Args::name.getKey().substr(0, Args::name.getKeyLen()),  //
                Args::name.getShortName(),                              //
                Args::description,                                      //
                Args::required,                                         //
                string_view(Args::typeName));
          } else {
            ret.emplace_back(
                Args::name.getKey().substr(0, Args::name.getKeyLen()),  //
                Args::name.getShortName(),                              //
                Args::description,                                      //
                false,                                                  //
                string_view(Args::typeName));
          }
        }(),
        ...);
    return ret;
  }
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
