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

using Argo::key;

auto [argc, argv] = createArgcArgv("./main", "--arg1", "23", "-h");

auto main() -> int {
  auto argo = Argo::Parser();
  auto parser = argo  //
                    .addArg<key("dummy", 'e'), int>(Argo::withDescription("Hello\nWorld!"))
                    .addArg<key("arg1"), int>(Argo::withDescription("Hello\nWorld!"))
                    .addHelp();

  parser.parse(argc, argv);

  std::println("{}", parser.getArg<key("arg1")>());
  // std::println("{}", parser.formatHelp());

  return 0;
}
