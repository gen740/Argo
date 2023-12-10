import Argo;

#include <benchmark/benchmark.h>

#include <cstdio>

#if CLI11_FOUND
#include <CLI/CLI.hpp>
#endif

#if argparse_FOUND
#include <argparse/argparse.hpp>
#endif

std::string options =
    "--arg1 1 2 3 4 5 6 7 8 --arg2 42.23 --arg3 --arg4 Hello,World -bc -d 3.14 "
    "-efgijklmn --arg18 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 "
    "21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 "
    "45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 "
    "69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 "
    "93 94 95 96 97 98 99";

static void emptyProgram(benchmark::State& state) {
  for (auto _ : state) {
#ifdef DEBUG_BUILD
    auto* fp = ::popen(("./bench-empty-debug " + options).c_str(), "r");
    ::pclose(fp);
#else
    auto* fp = ::popen(("./bench-empty-release " + options).c_str(), "r");
    ::pclose(fp);
#endif
  }
}

BENCHMARK(emptyProgram);

static void ArgoParser(benchmark::State& state) {
  for (auto _ : state) {
#ifdef DEBUG_BUILD
    auto* fp = ::popen(("./bench-argo-debug " + options).c_str(), "r");
    ::pclose(fp);
#else
    auto* fp = ::popen(("./bench-argo-release " + options).c_str(), "r");
    ::pclose(fp);
#endif
  }
}

BENCHMARK(ArgoParser);

#if CLI11_FOUND
static void CLI11Parser(benchmark::State& state) {
  for (auto _ : state) {
#ifdef DEBUG_BUILD
    auto* fp = ::popen(("./bench-cli11-debug " + options).c_str(), "r");
    ::pclose(fp);
#else
    auto* fp = ::popen(("./bench-cli11-release " + options).c_str(), "r");
    ::pclose(fp);
#endif
  }
}

BENCHMARK(CLI11Parser);
#endif

#if argparse_FOUND

static void argparseParser(benchmark::State& state) {
  for (auto _ : state) {
#ifdef DEBUG_BUILD
    auto* fp = ::popen(("./bench-argparse-debug " + options).c_str(), "r");
    ::pclose(fp);
#else
    auto* fp = ::popen(("./bench-argparse-release " + options).c_str(), "r");
    ::pclose(fp);
#endif
  }
}

BENCHMARK(argparseParser);

#endif

BENCHMARK_MAIN();
