import Argo;

#include <print>
#include <tuple>

using Argo::description;
using Argo::nargs;

auto main(int argc, char** argv) -> int {
  auto parser1 = Argo::Parser<"P1">()  //
                     .addArg<"p1a1", int>()
                     .addArg<"p1a2", int>();

  auto parser2 = Argo::Parser<"Parser2">()
                     .addArg<"p2a1", int>(description("Hello\nWorld!"))
                     .addArg<"p2a2", int>(description("Hello\nWorld!"))
                     .addHelp();

  auto parser3 = Argo::Parser<"Parser3">()
                     .addArg<"p3a1", int>(description("Hello\nWorld!"))
                     .addArg<"p3a2", int>(description("Hello\nWorld!"))
                     .addHelp();

  auto parser =                                             //
      Argo::Parser<"Parser">("./main", "Some description")  //
          .addParser<"cmd1">(parser1, description("subparser of 1\nsome help"))
          .addParser<"veryveryveryveryveryvery_longlonglongcmd">(
              parser2, description("subparser of 2\nsome help"))
          .addParser<"cmd3">(parser3)
          .addPositionalArg<"test", std::array<int, 3>>(
              description(R"(positional argument
long desc)"))
          .addPositionalArg<"ptest", std::string, Argo::Required>(
              description(R"(positional argument
long desc 2)"))
          .addArg<"test1,a", bool>(description("test1"))
          .addArg<"test2", int>(description("test2"))
          .addArg<"test3_very_very_very_long_long_long_long_option_name",
                  double>(description("test3"))
          .addArg<"test5,b", std::string, Argo::Required>(description("test4"))
          .addArg<"test6,c", const char*>(  //
              description("test5"))
          .addArg<"test7", std::string_view>(  //
              description("test6"))
          // nargs
          .addArg<"test8", bool, nargs(3), Argo::Required>(description("test7"))
          .addArg<"test9", int, nargs('+')>(description("test8"))
          .addArg<"test10", double, nargs('*')>(description("test9"))
          // STL
          .addArg<"test11", std::array<int, 3>>(
              description("test10\nsome help"))
          .addArg<"test12", std::array<std::string, 3>>(description("test11"))
          .addArg<"test13", std::vector<int>>(description("test12"))
          .addArg<"test14", std::vector<std::string>, Argo::nargs('+')>(
              description("test13"))
          .addArg<"test15", std::tuple<int, double, std::string>>(
              [](auto&&...) {})
          .addHelp<"help,h">();

  parser.parse(argc, argv);

  return 0;
}
