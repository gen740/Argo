import Argo;

#include <benchmark/benchmark.h>

#if CLI11_FOUND

#include <CLI/CLI.hpp>
#endif

const int argc = 17;
char* argv[argc] = {
    "./main",                                                                       //
    "--arg1", "1",     "2",      "3",      "4",           "5",    "6",   "7", "8",  // 10
    "--arg2", "42.23", "--arg3", "--arg4", "Hello,World", "-bcd", "3.14"            // 17
};

using Argo::arg;
using Argo::nargs;
using Argo::Parser;

static void ArgoParser1(benchmark::State& state) {
  for (auto _ : state) {
    auto argo = Parser<1>();
    auto parser = argo  //
                      .addArg<int, arg("arg1"), nargs(8)>()
                      .addArg<float, arg("arg2")>()
                      .addFlag<arg("arg3")>()
                      .addArg<std::string, arg("arg4"), nargs(1)>()
                      .addFlag<arg("arg5"), 'b'>()
                      .addFlag<arg("arg6"), 'c'>()
                      .addArg<float, arg("arg7"), 'd'>();
    parser.parse(argc, argv);
  }
}

BENCHMARK(ArgoParser1);

static void Cli11(benchmark::State& state) {
  for (auto _ : state) {
    CLI::App app{"App description"};

    std::array<int, 8> arg1;
    app.add_option("--arg1", arg1);
    float arg2;
    app.add_option("--arg2", arg2);
    app.add_flag("--arg3");
    std::string arg4;
    app.add_option("--arg4", arg4);

    app.add_flag("-b,--arg5");
    app.add_flag("-c,--arg6");

    float arg7;
    app.add_option("-d,--arg7", arg7);

    [&app]() {
      CLI11_PARSE(app, argc, argv);
      return 0;
    }();

    // auto argo = Parser<1>();
    // auto parser = argo  //
    //                   .addArg<int, arg("arg1"), nargs(8)>()
    //                   .addArg<float, arg("arg2")>()
    //                   .addFlag<arg("arg3")>()
    //                   .addArg<std::string, arg("arg4"), nargs(1)>()
    //                   .addFlag<arg("arg5"), 'b'>()
    //                   .addArg<float, arg("arg6"), 'c'>()
    //                   .addFlag<arg("arg7"), 'd'>();
    // parser.parse(argc, argv);
  }
}

BENCHMARK(Cli11);

BENCHMARK_MAIN();
