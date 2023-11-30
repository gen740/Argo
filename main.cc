import Argo;

#include <print>
#include <tuple>

template <typename... Args>
std::tuple<size_t, char**> createArgcArgv(Args... args) {
  const size_t N = sizeof...(Args);
  char** array = new char*[N];
  size_t i = 0;
  (..., (array[i++] = strdup(args)));
  return std::make_tuple(N, array);
}

auto [argc, argv] = createArgcArgv("./main", "cmd1", "--p1a1", "23");

auto main() -> int {
  auto parser1 = Argo::Parser<"P1">()  //
                     .addArg<"p1a1", int>()
                     .addArg<"p1a2", int>();
  auto parser2 = Argo::Parser<"Parser2">()
                     .addArg<"p2a1", int>(Argo::description("Hello\nWorld!"))
                     .addArg<"p2a2", int>(Argo::description("Hello\nWorld!"));

  auto parser = Argo::Parser<"Parser">()  //
                    .addParser<"cmd1">(parser1, Argo::description("subparser of 1\nsome help"))
                    .addParser<"cmd2">(parser2, Argo::description("subparser of 2\nsoemu"));

  parser.parse(argc, argv);
  std::println("{}", parser1.getArg<"p1a1">());

  return 0;
}
