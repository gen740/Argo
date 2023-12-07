module;

export module Argo:HelpGenerator;
import :std_module;

// generator start here

namespace Argo {

struct ArgInfo {
  std::string_view name;
  char shortName;
  std::string_view description;
  bool required;
  std::string typeName;
};

export template <class Args>
struct HelpGenerator {};

export template <class... Args>
struct HelpGenerator<std::tuple<Args...>> {
  static auto generate() -> std::vector<ArgInfo> {
    std::vector<ArgInfo> ret;
    (
        [&ret]<class T>() {
          ret.emplace_back(
              std::string_view(Args::name).substr(0, Args::name.nameLen),
              Args::name.shortName, Args::description, Args::required,
              Args::typeName);
        }.template operator()<Args>(),
        ...);
    return ret;
  }
};

struct SubCommandInfo {
  std::string_view name;
  std::string_view description;
};

export template <class T>
auto SubParserInfo(T subparsers) {
  std::vector<SubCommandInfo> ret{};
  if constexpr (!std::is_same_v<T, std::tuple<>>) {
    std::apply(
        [&ret]<class... Parser>(Parser... parser) {
          (..., ret.emplace_back(parser.name, parser.description));
        },
        subparsers);
  }
  return ret;
};

}  // namespace Argo

// generator end here
