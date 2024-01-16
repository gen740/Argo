import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

using Argo::description;
using Argo::nargs;
using Argo::Parser;

TEST(ArgoTest, Help) {
  auto parser1 = Parser<"P1">()  //
                     .addArg<"p1a1", int>();

  auto parser2 = Parser<"Parser2">()
                     .addArg<"p2a0", int>(description("Hello\nWorld!"))
                     .addHelp();

  auto parser3 = Parser<"Parser2">()
                     .addArg<"p3a0", int>(description("Hello\nWorld!"))
                     .addHelp();

  auto parser =                                       //
      Parser<"Parser">("./main", "Some description")  //
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

  const auto* expected_help = R"(Some description

Usage:
  ./main -b [<STRING>] [options...] [test] long_long_positional_arg long_long_positionalarg {cmd1,veryveryvery_longlongcmd,veryveryverylonglongcmd}

Subcommands:
  cmd1                    subparser1
  veryveryvery_longlongcmd
                          subparser2
                          some help
  veryveryverylonglongcmd subparser of 3
                          some help

Positional Argument:
  test                    Positional argument help
  long_long_positional_arg
                          Positional argument help
                          multyple help
  long_long_positionalarg help

Options:
  -a,--p1 [<BOOL>]        test1
     --p2 [<NUMBER>]      multiple
                          lines
                          description
     --p3_name_ [<FLOAT>] multiple
                          lines
                          description
  -b,--p4_name_ [<STRING>]
                          test4
  -h,--help               Print help information
)";

  EXPECT_EQ(parser.formatHelp(true), expected_help);
}

TEST(ArgoTest, HelpFlag) {
  auto [argc, argv] = createArgcArgv(  //
      "./main",                        //
      "--help");
  auto argo = Parser<"Help flag">("program");
  auto parser = argo.addArg<"arg0,k", int>()
                    .addArg<"arg1,a", int, nargs('+')>()
                    .addHelp();

  testing::internal::CaptureStdout();
  EXPECT_EXIT(parser.parse(argc, argv.get()), testing::ExitedWithCode(0), "");
  testing::internal::GetCapturedStdout();
}

TEST(ArgoTest, ShortHelpFlag) {
  auto [argc, argv] = createArgcArgv(  //
      "./main",                        //
      "-h");
  auto argo = Parser<"Help short flag">("program");
  auto parser = argo.addArg<"arg0,k", int>()
                    .addArg<"arg1,a", int, nargs('+')>()
                    .addHelp();

  testing::internal::CaptureStdout();
  EXPECT_EXIT(parser.parse(argc, argv.get()), testing::ExitedWithCode(0), "");
  testing::internal::GetCapturedStdout();
}

TEST(ArgoTest, ChangeHelpFlag) {
  auto [argc, argv] = createArgcArgv(  //
      "./main",                        //
      "-g");
  auto argo = Parser<"Help Change Flag">("program");
  auto parser = argo.addArg<"arg0,k", int>()
                    .addArg<"arg1,a", int, nargs('+')>()
                    .addHelp<"generate_help,g">();

  testing::internal::CaptureStdout();
  EXPECT_EXIT(parser.parse(argc, argv.get()), testing::ExitedWithCode(0), "");
  testing::internal::GetCapturedStdout();
}
