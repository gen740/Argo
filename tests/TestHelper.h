#pragma once

#include <memory>
#include <print>

template <typename... Args>
auto createArgcArgv(Args... args) -> std::tuple<int, std::unique_ptr<char*[]>> {
  const size_t N = sizeof...(Args);
  // char** array = new char*[N];
  auto array = std::make_unique<char*[]>(N);
  size_t i = 0;
  (..., (array[i++] = strdup(args)));
  return std::make_tuple(N, std::move(array));
}
