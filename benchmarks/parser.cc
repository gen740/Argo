import Argo;

#include <benchmark/benchmark.h>

#if CLI11_FOUND
#include <CLI/CLI.hpp>
#endif

#if argparse_FOUND
#include <argparse/argparse.hpp>
#endif

template <typename... Args>
std::tuple<int, char**> createArgcArgv(Args... args) {
  const size_t N = sizeof...(Args);
  char** array = new char*[N];
  size_t i = 0;
  (..., (array[i++] = strdup(args)));
  return std::make_tuple(static_cast<int>(N), array);
}

auto [argc, argv] = createArgcArgv(                    //
    "./main",                                          //
    "--arg1", "1", "2", "3", "4", "5", "6", "7", "8",  //
    "--arg2", "42.23",                                 //
    "--arg3",                                          //
    "--arg4", "Hello,World",                           //
    "-bc", "-d", "3.14",                               // 17
    "-efgijklmn",                                      //
    "--arg18",                                         //
    "0", "1", "2", "3", "4",                           //
    "--arg19",                                         //
    "0.1", "0.2", "0.3"                                //
);

using Argo::nargs;
using Argo::Parser;

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
                      .addArg<"arg18", int, nargs('+')>()
                      .addArg<"arg19", double, nargs(3)>();
    parser.parse(argc, argv);
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

    std::vector<double> arg19;
    app.add_option("--arg19", arg19);

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
    program.add_argument("--arg18")
        .nargs(argparse::nargs_pattern::at_least_one)
        .scan<'d', int>();
    program.add_argument("--arg19").nargs(3).scan<'g', float>();

    program.parse_args(argc, argv);

    program.get<std::vector<int>>("--arg1");
    program.get<double>("--arg2");
    program.get<bool>("--arg3");
  }
}

BENCHMARK(argparseParser);

#endif

BENCHMARK_MAIN();
