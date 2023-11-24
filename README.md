# Argo
C++ mordern argument parser, using C++ module and metaprogramming.


## Example
```cpp
import Argo;
import std;

// suppose ./main --arg1 42 --arg3 "Hello,World"
auto main(int argc, char* argv[]) -> int {
  auto argo = Argo::Parser();

  auto parser = argo
                    .addArg<int, Argo::arg("arg1")>()
                    .addArg<float, Argo::arg("arg2")>(53.4)
                    .addArg<std::string, Argo::arg("arg3")>();

  parser.parse(argc, argv);

  std::println("{}", parser.getArg<Argo::arg("arg1")>()); // 42
  std::println("{}", parser.getArg<Argo::arg("arg2")>()); // 53.4 (default value)
  std::println("{}", parser.getArg<Argo::arg("arg3")>()); // Hello,World
  // std::println("{}", parser.getArg<Argo::arg("arg4")>()); // error

  // Static Type
  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg1")>()), int>);
  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg2")>()), float>);
  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg3")>()), std::string>);

  return 0;
}
```


