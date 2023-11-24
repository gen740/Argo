import Argo;
import std_module;

auto main(int argc, char* argv[]) -> int {
  auto argo = Argo::Parser();

  auto parser = argo                                   //
                    .addArg<int, Argo::arg("arg1")>()  //
                    .addArg<std::string, Argo::arg("arg2")>();

  parser.parse(argc, argv);

  std::println("{}", parser.getArg<Argo::arg("arg1")>());
  std::println("{}", parser.getArg<Argo::arg("arg2")>());

  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg1")>()), int>);
  static_assert(std::is_same_v<decltype(parser.getArg<Argo::arg("arg2")>()),
                               std::string>);

  return 0;
}
