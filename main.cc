import Argo;
import std_module;

auto main(int argc, char* argv[]) -> int {
  auto argo = Argo::Argo();

  auto parser = argo                                           //
                    .addArg<int, Argo::arg("foo")>()           //
                    .addArg<std::string, Argo::arg("bar")>();  //

  parser.parse(argc, argv);

  std::println("{}", parser.getArg<Argo::arg("foo")>());
  std::println("{}", parser.getArg<Argo::arg("bar")>());

  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::arg("foo")>()), int>);
  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::arg("bar")>()), std::string>);

  return 0;
}
