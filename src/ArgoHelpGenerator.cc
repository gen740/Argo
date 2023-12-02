module;

export module Argo:HelpGenerator;
import :std_module;

export namespace Argo {

struct ArgInfo {
  std::string_view name;
  char shortName;
  std::string_view description;
};

template <class Args>
struct HelpGenerator {};

template <class... Args>
struct HelpGenerator<std::tuple<Args...>> {
  static auto generate() -> std::vector<ArgInfo> {
    std::vector<ArgInfo> ret;
    (
        [&ret]<class T>() {
          ret.emplace_back(std::string_view(Args::name).substr(0, Args::name.nameLen),
                           Args::name.shortName, Args::description);
        }.template operator()<Args>(),
        ...);
    return ret;
  }
};

struct SubCommandInfo {
  std::string_view name;
  std::string_view description;
};

struct SubParserHelpGenerator {
  template <class SubParsers, std::size_t... Numbers>
  static auto eval(SubParsers tuple, std::index_sequence<Numbers...>) {
    std::vector<SubCommandInfo> ret{};
    (
        [&tuple, &ret]<std::size_t N>() {
          ret.emplace_back(std::get<N>(tuple).name, std::get<N>(tuple).description);
        }.template operator()<Numbers>(),
        ...);
    return ret;
  }

  template <class... SubParsers>
  static auto generate(std::tuple<SubParsers...> tuple) {
    return eval(tuple, std::make_index_sequence<sizeof...(SubParsers)>());
  }
};

}  // namespace Argo
