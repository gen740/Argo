import Argo;

#include <benchmark/benchmark.h>

#if CLI11_FOUND
#include <CLI/CLI.hpp>
#endif

#if argparse_FOUND
#include <argparse/argparse.hpp>
#endif

const int argc = 18;
char* argv[argc] = {
    "./main",                                                                       //
    "--arg1", "1",     "2",      "3",      "4",           "5",   "6",  "7",   "8",  // 10
    "--arg2", "42.23", "--arg3", "--arg4", "Hello,World", "-bc", "-d", "3.14"       // 17
};

using Argo::key;
using Argo::nargs;
using Argo::Parser;

static void ArgoParser(benchmark::State& state) {
  for (auto _ : state) {
    auto argo = Parser<1>();
    auto parser = argo  //
                      .addArg<int, key("arg1"), nargs(8)>()
                      .addArg<float, key("arg2")>()
                      .addFlag<key("arg3")>()
                      .addArg<std::string, key("arg4"), nargs(1)>()
                      .addFlag<key("arg5"), 'b'>()
                      .addFlag<key("arg6"), 'c'>()
                      .addArg<float, key("arg7"), 'd'>();
    parser.parse(argc, argv);
  }
}

BENCHMARK(ArgoParser);

#if CLI11_FOUND
static void CLI11Parser(benchmark::State& state) {
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
  }
}

BENCHMARK(CLI11Parser);
#endif

#if argparse_FOUND
static void argparseParser(benchmark::State& state) {
  for (auto _ : state) {
    argparse::ArgumentParser program("program_name");

    program.add_argument("--arg1").nargs(8).scan<'d', int>();
    program.add_argument("--arg2").scan<'g', double>();
    program.add_argument("--arg3").default_value(false);
    program.add_argument("--arg4").nargs(1);
    program.add_argument("-b", "--arg5").default_value(false);
    program.add_argument("-c", "--arg6").default_value(false);
    program.add_argument("-d", "--arg7");

    program.parse_args(argc, argv);

    program.get<std::vector<int>>("--arg1");
    program.get<double>("--arg2");
    program.get<bool>("--arg3");
  }
}

BENCHMARK(argparseParser);

#endif

BENCHMARK_MAIN();
