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
          ret.emplace_back(std::string_view(Args::name), Args::name.shortName, Args::description);
        }.template operator()<Args>(),
        ...  //
    );
    return ret;
  }
};

}  // namespace Argo
