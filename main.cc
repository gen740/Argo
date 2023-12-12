import Argo;

#include <print>
#include <tuple>

using Argo::description;
using Argo::nargs;

auto main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> int {
  __asm__("# Creating parser");
  auto parser1 = Argo::Parser<"P1">()  //
                     .addArg<"p1a1", int>()
                     .addArg<"p1a2", int>();
  __asm__("# Created parser");

  auto parser2 = Argo::Parser<"Parser2">()
                     .addArg<"p2a1", int>(description("Hello\nWorld!"))
                     .addArg<"p2a2", int>(description("Hello\nWorld!"))
                     .addHelp();

  auto parser =                                             //
      Argo::Parser<"Parser">("./main", "Some description")  //
          .addParser<"cmd1">(parser1, description("subparser of 1\nsome help"))
          .addParser<"cmd2">(parser2, description("subparser of 2\nsome help"))
          .addPositionalArg<"test", std::array<int, 3>>(
              description(R"(positional argument\nlong desc)"))
          .addArg<"test1", bool>()
          .addArg<"test2", int>()
          .addArg<"test3", double>()
          .addArg<"test5", std::string>()
          .addArg<"test6", const char*>()
          .addArg<"test7", std::string_view>()
          // nargs
          .addArg<"test8", bool, nargs(3)>()
          .addArg<"test9", int, nargs('+')>()
          .addArg<"test10", double, nargs('*')>()
          // STL
          .addArg<"test11", std::array<int, 3>>()
          .addArg<"test12", std::array<std::string, 3>>()
          .addArg<"test13", std::vector<int>>()
          .addArg<"test14", std::vector<std::string>, Argo::nargs('+')>()
          .addArg<"test15", std::tuple<int, double, std::string>>(
              [](auto&&...) {})
          .addHelp<"help,h">();

  __asm__("# Parse_ Start");
  parser.parse(argc, argv);
  __asm__("# Parse_ End");

  return 0;
}
