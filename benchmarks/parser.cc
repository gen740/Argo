import Argo;

#include <benchmark/benchmark.h>

#if CLI11_FOUND
#include <CLI/CLI.hpp>
#endif

#if argparse_FOUND
#include <argparse/argparse.hpp>
#endif

template <typename... Args>
std::tuple<size_t, char**> createArgcArgv(Args... args) {
  const size_t N = sizeof...(Args);
  char** array = new char*[N];
  size_t i = 0;
  (..., (array[i++] = strdup(args)));
  return std::make_tuple(N, array);
}

auto [argc, argv] = createArgcArgv(                              //
    "./main",                                                    //
    "--arg1", "1", "2", "3", "4", "5", "6", "7", "8",            //
    "--arg2", "42.23",                                           //
    "--arg3",                                                    //
    "--arg4", "Hello,World",                                     //
    "-bc", "-d", "3.14",                                         // 17
    "-efgijklmn",                                                //
    "--arg18",                                                   //
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",            //
    "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",  //
    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",  //
    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",  //
    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",  //
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",  //
    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69",  //
    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79",  //
    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89",  //
    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99"   //
);

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
                      .addArg<float, key("arg7"), 'd'>()
                      .addFlag<key("arg8"), 'e'>()
                      .addFlag<key("arg9"), 'f'>()
                      .addFlag<key("arg10"), 'g'>()
                      .addFlag<key("arg12"), 'i'>()
                      .addFlag<key("arg13"), 'j'>()
                      .addFlag<key("arg14"), 'k'>()
                      .addFlag<key("arg15"), 'l'>()
                      .addFlag<key("arg16"), 'm'>()
                      .addFlag<key("arg17"), 'n'>()
                      .addArg<int, key("arg18"), nargs('+')>();
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

    [&app]() {
      CLI11_PARSE(app, argc, argv);
      return 0;
    }();
  }
}

BENCHMARK(CLI11Parser);
#endif

#if argparse_FOUND
argparse::ArgumentParser program("program_name");

static void argparseParser(benchmark::State& state) {
  for (auto _ : state) {
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

    program.parse_args(argc, argv);

    program.get<std::vector<int>>("--arg1");
    program.get<double>("--arg2");
    program.get<bool>("--arg3");
  }
}

BENCHMARK(argparseParser);

#endif

BENCHMARK_MAIN();
