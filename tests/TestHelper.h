#pragma once

#include <format>
#include <print>
#include <string>
#include <type_traits>

template <typename... Args>
std::tuple<int, char**> createArgcArgv(Args... args) {
  const size_t N = sizeof...(Args);
  char** array = new char*[N];
  size_t i = 0;
  (..., (array[i++] = strdup(args)));
  return std::make_tuple(N, array);
}
