import Argo;

#include <print>

using Argo::description;
using Argo::nargs;

auto main(int argc, char** argv) -> int {
  auto parser1 = Argo::Parser<"P1">()  //
                     .addArg<"p1a1", int>();

  auto parser2 = Argo::Parser<"Parser2">()
                     .addArg<"p2a0", int>(description("Hello\nWorld!"))
                     .addHelp();

  auto parser3 = Argo::Parser<"Parser2">()
                     .addArg<"p3a0", int>(description("Hello\nWorld!"))
                     .addHelp();

  auto parser =                                             //
      Argo::Parser<"Parser">("./main", "Some description")  //
          .addParser<"cmd1">(parser1, description("subparser1"))
          .addParser<"veryveryvery_longlongcmd">(
              parser2, description("subparser2\nsome help"))
          .addParser<"veryveryverylonglongcmd">(
              parser3, description("subparser of 3\nsome help"))
          .addPositionalArg<"test", std::array<int, 3>>(
              description("Positional argument help"))
          .addPositionalArg<"long_long_positional_arg", std::string,
                            Argo::Required>(
              description("Positional argument help\nmultyple help"))
          .addPositionalArg<"long_long_positionalarg", std::string,
                            Argo::Required>(description("help"))
          .addArg<"p1,a", bool>(description("test1"))
          .addArg<"p2", int>(description("multiple\nlines\ndescription"))
          .addArg<"p3_name_", double>(
              description("multiple\nlines\ndescription"))
          .addArg<"p4_name_,b", std::string, Argo::Required>(
              description("test4"))
          .addHelp<"help,h">();

  parser.parse(argc, argv);
  return 0;
}
