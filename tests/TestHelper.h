#pragma once

#include <format>
#include <memory>
#include <print>
#include <string>
#include <type_traits>

template <typename... Args>
std::tuple<int, std::unique_ptr<char*[]>> createArgcArgv(Args... args) {
  const size_t N = sizeof...(Args);
  // char** array = new char*[N];
  auto array = std::make_unique<char*[]>(N);
  size_t i = 0;
  (..., (array[i++] = strdup(args)));
  return std::make_tuple(N, std::move(array));
}
