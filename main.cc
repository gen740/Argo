import Argo;
#include <iostream>
#include <tuple>



template <typename... Args>
std::tuple<size_t, char**> createArgcArgv(Args... args) {
  const size_t N = sizeof...(Args);
  char** array = new char*[N];
  size_t i = 0;
  (..., (array[i++] = strdup(args)));
  return std::make_tuple(N, array);
}

auto main() -> int {

  // auto argo = Argo::Parser();
  // auto parser = argo.addArg<

  return 0;
}
