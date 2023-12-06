#include "./include/Argo.hh"

// suppose ./main --arg1 42 --arg3 "Hello,World"
auto main(int argc, char* argv[]) -> int {
  auto parser = Argo::Parser()  //
                    .addArg<"arg1", int>()
                    .addArg<"arg2", float>(Argo::explicitDefault(12.34))
                    .addArg<"arg3", std::string>();

  parser.parse(argc, argv);

  std::println("{}", parser.getArg<"arg1">());  // 42
  std::println("{}", parser.getArg<"arg2">());  // 12.34 (default value)
  std::println("{}", parser.getArg<"arg3">());  // Hello,World

  // Static Typing
  static_assert(std::is_same_v<decltype(parser.getArg<"arg1">()), int>);
  static_assert(std::is_same_v<decltype(parser.getArg<"arg2">()), float>);
  static_assert(std::is_same_v<decltype(parser.getArg<"arg3">()), std::string>);

  return 0;
}
