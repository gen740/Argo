import Argo;
import std_module;

auto main(int argc, char* argv[]) -> int {
  auto argo = Argo::Parser();

  auto parser = argo                                           //
                    .addArg<int, Argo::arg("foo")>()           //
                    .addArg<float, Argo::arg("foo2")>()        //
                    .addArg<float, Argo::arg("foo3")>(53.4)    //
                    .addArg<std::string, Argo::arg("bar")>();  //

  parser.parse(argc, argv);

  std::println("{}", parser.getArg<Argo::arg("foo")>());
  std::println("{}", parser.getArg<Argo::arg("foo2")>());
  std::println("{}", parser.getArg<Argo::arg("foo3")>());
  std::println("{}", parser.getArg<Argo::arg("bar")>());

  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::arg("foo")>()), int>);
  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::arg("bar")>()), std::string>);

  return 0;
}
