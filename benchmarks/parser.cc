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

auto [argc, argv] = createArgcArgv(
    "./main",                           //
    "--arg1", "1",                      // Integer
    "--arg2", "42.23",                  // Floating-point number
    "--arg3", "true",                   // Boolean value (true/false)
    "--arg4", "Hello, World",           // String
    "--arg5", "-123",                   // Negative integer
    "--arg6", "3.14e-10",               // Scientific notation floating-point
    "--arg7", "false",                  // Boolean value (false)
    "--arg8", "\"quoted string\"",      // Quoted string
    "--arg9", "0x1A",                   // Hexadecimal number
    "--arg10", "010",                   // Octal number
    "--arg11", "binary", "101010",      // Binary string
    "--arg12", "multi", "word", "arg",  // Multi-word argument
    "--arg13", "1234567890",            // Long integer
    "--arg14", "2.718281828459045",     // High precision floating-point
    "--arg15", "short",                 // Short string
    "--arg16", "4.2",                   // Floating-point number
    "--arg17", "0",                     // Zero
    "--arg18", "negative", "-42",       // Negative integer
    "--arg19", "verylongstringthatdoesnotfitinasmallbox",  // Long string
    "--arg20", "255",                  // Integer (max value of a byte)
    "--arg21", "3.14159",              // Approximation of Pi
    "--arg22", "empty", "",            // Empty string
    "--arg23", "0xFF",                 // Hexadecimal number
    "--arg24", "0755",                 // Octal number (UNIX permission format)
    "--arg25", "2023-12-31",           // Date format string
    "--arg26", "100",                  // Scientific notation
    "--arg27", "one", "two", "three",  // Multiple values
    "--arg28", "newline\ncharacter",   // String with newline character
    "--arg29", "tab\tcharacter",       // String with tab character
    "--arg30", "end"                   // Last argument
);

using Argo::nargs;
using Argo::Parser;

static void ArgoParser(benchmark::State& state) {
  for (auto _ : state) {
    auto parser = Parser<1>()  //
                      .addArg<"arg1", int>()
                      .addArg<"arg2", double>()
                      .addArg<"arg3", bool>()
                      .addArg<"arg4", std::string>()
                      .addArg<"arg5", int>()
                      .addArg<"arg6", double>()
                      .addArg<"arg7", bool>()
                      .addArg<"arg8", std::string>()
                      .addArg<"arg9", int>()
                      .addArg<"arg10", int>()
                      .addArg<"arg11", std::tuple<std::string, int>>()
                      .addArg<"arg12", std::array<std::string, 3>>()
                      .addArg<"arg13", int64_t>()
                      .addArg<"arg14", double>()
                      .addArg<"arg15", std::string>()
                      .addArg<"arg16", double>()
                      .addArg<"arg17", int>()
                      .addArg<"arg18", std::tuple<std::string, int>>()
                      .addArg<"arg19", std::string>()
                      .addArg<"arg20", uint8_t>()
                      .addArg<"arg21", double>()
                      .addArg<"arg22", std::string, nargs(2)>()
                      .addArg<"arg23", int>()
                      .addArg<"arg24", int>()
                      .addArg<"arg25", std::string>()
                      .addArg<"arg26", int>()
                      .addArg<"arg27", std::string, nargs(3)>()
                      .addArg<"arg28", std::string>()
                      .addArg<"arg29", std::string>()
                      .addArg<"arg30", std::string>();
    parser.parse(argc, argv);
    parser.resetArgs();
  }
}

BENCHMARK(ArgoParser);

#if CLI11_FOUND
static void CLI11Parser(benchmark::State& state) {
  for (auto _ : state) {
    auto app = CLI::App{"App description"};

    int arg1;
    app.add_option("--arg1", arg1);
    double arg2;
    app.add_option("--arg2", arg2);
    bool arg3;
    app.add_option("--arg3", arg3);
    std::string arg4;
    app.add_option("--arg4", arg4);
    int arg5;
    app.add_option("--arg5", arg5);
    double arg6;
    app.add_option("--arg6", arg6);
    bool arg7;
    app.add_option("--arg7", arg7);
    std::string arg8;
    app.add_option("--arg8", arg8);
    int arg9;
    app.add_option("--arg9", arg9);
    int arg10;
    app.add_option("--arg10", arg10);
    std::tuple<std::string, int> arg11;
    app.add_option("--arg11", arg11);
    std::array<std::string, 3> arg12;
    app.add_option("--arg12", arg12);
    int64_t arg13;
    app.add_option("--arg13", arg13);
    double arg14;
    app.add_option("--arg14", arg14);
    std::string arg15;
    app.add_option("--arg15", arg15);
    double arg16;
    app.add_option("--arg16", arg16);
    int arg17;
    app.add_option("--arg17", arg17);
    std::tuple<std::string, int> arg18;
    app.add_option("--arg18", arg18);
    std::string arg19;
    app.add_option("--arg19", arg19);
    uint8_t arg20;
    app.add_option("--arg20", arg20);
    double arg21;
    app.add_option("--arg21", arg21);
    std::vector<std::string> arg22;
    app.add_option("--arg22", arg22)->expected(2);
    int arg23;
    app.add_option("--arg23", arg23);
    int arg24;
    app.add_option("--arg24", arg24);
    std::string arg25;
    app.add_option("--arg25", arg25);
    int arg26;
    app.add_option("--arg26", arg26);
    std::vector<std::string> arg27;
    app.add_option("--arg27", arg27)->expected(3);
    std::string arg28;
    app.add_option("--arg28", arg28);
    std::string arg29;
    app.add_option("--arg29", arg29);
    std::string arg30;
    app.add_option("--arg30", arg30);

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
  auto program = argparse::ArgumentParser("program_name");
  for (auto _ : state) {
    program.add_argument("--arg1", "-a1").scan<'i', int>();
    program.add_argument("--arg2", "-a2").scan<'g', double>();
    program.add_argument("--arg3", "-a3");
    program.add_argument("--arg4", "-a4");
    program.add_argument("--arg5", "-a5").scan<'i', int>();
    program.add_argument("--arg6", "-a6").scan<'g', double>();
    program.add_argument("--arg7", "-a7");
    program.add_argument("--arg8", "-a8");
    program.add_argument("--arg9", "-a9").scan<'i', int>();
    program.add_argument("--arg10", "-a10").scan<'i', int>();
    program.add_argument("--arg11", "-a11").nargs(2);
    program.add_argument("--arg12", "-a12").nargs(3);
    program.add_argument("--arg13", "-a13").scan<'i', int64_t>();
    program.add_argument("--arg14", "-a14").scan<'g', double>();
    program.add_argument("--arg15", "-a15");
    program.add_argument("--arg16", "-a16").scan<'g', double>();
    program.add_argument("--arg17", "-a17").scan<'i', int>();
    program.add_argument("--arg18", "-a18").nargs(2);
    program.add_argument("--arg19", "-a19");
    program.add_argument("--arg20", "-a20").scan<'i', uint8_t>();
    program.add_argument("--arg21", "-a21").scan<'g', double>();
    program.add_argument("--arg22", "-a22").nargs(2);
    program.add_argument("--arg23", "-a23").scan<'i', int>();
    program.add_argument("--arg24", "-a24").scan<'i', int>();
    program.add_argument("--arg25", "-a25");
    program.add_argument("--arg26", "-a26").scan<'i', int>();
    program.add_argument("--arg27", "-a27").nargs(3);
    program.add_argument("--arg28", "-a28");
    program.add_argument("--arg29", "-a29");
    program.add_argument("--arg30", "-a30");
    program.parse_args(argc, argv);
    benchmark::DoNotOptimize(program.get<int>("--arg1"));
    benchmark::DoNotOptimize(program.get<double>("--arg2"));
    benchmark::DoNotOptimize(program.get<std::string>("--arg3"));
    benchmark::DoNotOptimize(program.get<std::string>("--arg4"));
    benchmark::DoNotOptimize(program.get<int>("--arg5"));
    benchmark::DoNotOptimize(program.get<double>("--arg6"));
    benchmark::DoNotOptimize(program.get<std::string>("--arg7"));
    benchmark::DoNotOptimize(program.get<std::string>("--arg8"));
    benchmark::DoNotOptimize(program.get<int>("--arg9"));
    benchmark::DoNotOptimize(program.get<int>("--arg10"));
    benchmark::DoNotOptimize(program.get<std::vector<std::string>>("--arg11"));
    benchmark::DoNotOptimize(program.get<std::vector<std::string>>("--arg12"));
    benchmark::DoNotOptimize(program.get<int64_t>("--arg13"));
    benchmark::DoNotOptimize(program.get<double>("--arg14"));
    benchmark::DoNotOptimize(program.get<std::string>("--arg15"));
    benchmark::DoNotOptimize(program.get<double>("--arg16"));
    benchmark::DoNotOptimize(program.get<int>("--arg17"));
    benchmark::DoNotOptimize(program.get<std::vector<std::string>>("--arg18"));
    benchmark::DoNotOptimize(program.get<std::string>("--arg19"));
    benchmark::DoNotOptimize(program.get<uint8_t>("--arg20"));
    benchmark::DoNotOptimize(program.get<double>("--arg21"));
    benchmark::DoNotOptimize(program.get<std::vector<std::string>>("--arg22"));
    benchmark::DoNotOptimize(program.get<int>("--arg23"));
    benchmark::DoNotOptimize(program.get<int>("--arg24"));
    benchmark::DoNotOptimize(program.get<std::string>("--arg25"));
    benchmark::DoNotOptimize(program.get<int>("--arg26"));
    benchmark::DoNotOptimize(program.get<std::vector<std::string>>("--arg27"));
    benchmark::DoNotOptimize(program.get<std::string>("--arg28"));
    benchmark::DoNotOptimize(program.get<std::string>("--arg29"));
    benchmark::DoNotOptimize(program.get<std::string>("--arg30"));
  }
}

BENCHMARK(argparseParser);

#endif

BENCHMARK_MAIN();
