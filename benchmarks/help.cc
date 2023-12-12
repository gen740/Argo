import Argo;

#include <benchmark/benchmark.h>

#include <fstream>

#if CLI11_FOUND
#include <CLI/CLI.hpp>
#endif

#if argparse_FOUND
#include <argparse/argparse.hpp>
#endif

using Argo::nargs;
using Argo::Parser;

auto nullout = std::ofstream("/dev/null");

static void ArgoParser(benchmark::State& state) {
  for (auto _ : state) {
    auto parser = Parser<1>()  //
                      .addArg<"arg1", int, nargs(8)>()
                      .addArg<"arg2", float>()
                      .addFlag<"arg3">()
                      .addArg<"arg4", std::string, nargs(1)>()
                      .addFlag<"arg5,b">()
                      .addFlag<"arg6,c">()
                      .addArg<"arg7,d", float>()
                      .addFlag<"arg8,e">()
                      .addFlag<"arg9,f">()
                      .addFlag<"arg10,g">()
                      .addFlag<"arg12,i">()
                      .addFlag<"arg13,j">()
                      .addFlag<"arg14,k">()
                      .addFlag<"arg15,l">()
                      .addFlag<"arg16,m">()
                      .addFlag<"arg17,n">()
                      .addArg<"arg18", int, nargs('+')>();
    nullout << parser.formatHelp() << '\n';
    parser.resetArgs();
  }
}

BENCHMARK(ArgoParser);

#if CLI11_FOUND
static void CLI11Parser(benchmark::State& state) {
  for (auto _ : state) {
    auto app = CLI::App{"App description"};

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

    app.add_flag("-e,--arg8");
    app.add_flag("-f,--arg9");
    app.add_flag("-g,--arg10");
    app.add_flag("-i,--arg12");
    app.add_flag("-j,--arg13");
    app.add_flag("-k,--arg14");
    app.add_flag("-l,--arg15");
    app.add_flag("-m,--arg16");
    app.add_flag("-n,--arg17");

    std::vector<int> arg18;
    app.add_option("--arg18", arg18);

    nullout << app.help() << '\n';
  }
}

BENCHMARK(CLI11Parser);
#endif

#if argparse_FOUND

static void argparseParser(benchmark::State& state) {
  for (auto _ : state) {
    auto program = argparse::ArgumentParser("program_name");
    program.add_argument("--arg1").nargs(8).scan<'d', int>();
    program.add_argument("--arg2").scan<'g', double>();
    program.add_argument("--arg3").default_value(false);
    program.add_argument("--arg4").nargs(1);
    program.add_argument("-b", "--arg5").default_value(false);
    program.add_argument("-c", "--arg6").default_value(false);
    program.add_argument("-d", "--arg7");
    program.add_argument("-e", "--arg8").default_value(false);
    program.add_argument("-f", "--arg9").default_value(false);
    program.add_argument("-g", "--arg10").default_value(false);
    program.add_argument("-i", "--arg11").default_value(false);
    program.add_argument("-j", "--arg12").default_value(false);
    program.add_argument("-k", "--arg13").default_value(false);
    program.add_argument("-l", "--arg14").default_value(false);
    program.add_argument("-m", "--arg15").default_value(false);
    program.add_argument("-n", "--arg16").default_value(false);
    program.add_argument("--arg18").nargs(100).scan<'d', int>();

    nullout << program << '\n';
  }
}

BENCHMARK(argparseParser);

#endif

BENCHMARK_MAIN();
