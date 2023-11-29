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

auto main() -> int {
  // auto[argc, argv] = createArgcArgv("./main", "--arg1", "23")

  auto argo = Argo::Parser();
  auto parser = argo  //
                    .addArg<key("arg1"), int>(Argo::withDescription("Hello\nWorld!"));

  std::println("{}", parser.formatHelp());

  return 0;
}
