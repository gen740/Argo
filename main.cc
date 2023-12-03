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

auto main(int argc, char** argv) -> int {
  auto parser1 = Argo::Parser<"P1">()  //
                     .addArg<"p1a1", int>()
                     .addArg<"p1a2", int>();

  auto parser2 = Argo::Parser<"Parser2">()
                     .addArg<"p2a1", int>(Argo::description("Hello\nWorld!"))
                     .addArg<"p2a2", int>(Argo::description("Hello\nWorld!"))
                     .addHelp();

  auto parser = Argo::Parser<"Parser">("Test Program", "Some description")  //
                    .addParser<"cmd1">(parser1, Argo::description("subparser of 1\nsome help"))
                    .addParser<"cmd2">(parser2, Argo::description("subparser of 2\nsome help"))
                    .addArg<"test1,a", int>(Argo::description(
                        "Some long long long long long long long\nlong long long description"))
                    .addArg<"test2", std::array<int, 3>>()
                    .addArg<"test3", std::array<char, 2>>()
                    .addArg<"test4", std::vector<int>>()
                    .addArg<"test5", std::vector<int>, Argo::nargs('+')>()
                    .addArg<"test6", std::tuple<int, std::string>>()
                    .addArg<"test7", int>()
                    .addArg<"test8", bool>()
                    .addArg<"test9", float>()
                    .addArg<"test10", const char*>()
                    .addHelp();

  parser.parse(argc, argv);

  return 0;
}
