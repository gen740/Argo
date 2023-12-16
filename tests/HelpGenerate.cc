import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

TEST(ArgoTest, Help) {
  auto argo = Argo::Parser<"Help 1">("program");
  auto parser = argo.addArg<"arg0,k", int>()
                    .addArg<"arg1,a", int, Argo::nargs('+')>()
                    .addArg<"arg2", int, Argo::nargs('+')>(
                        Argo::description("This is arg2"))
                    .addArg<"arg3", std::tuple<int, double, std::string> >(
                        Argo::description("This is arg3\nlong description"));

  const auto* expect_help = R"(
Usage:
  program [options...]

Options:
  -k, --arg0 [<NUMBER>]
  -a, --arg1 <NUMBER,...>
      --arg2 <NUMBER,...>                       This is arg2
      --arg3 <NUMBER,FLOAT,STRING>              This is arg3
                                                long description
)";
  EXPECT_EQ(parser.formatHelp(true), expect_help);
}

TEST(ArgoTest, HelpFlag) {
  auto [argc, argv] = createArgcArgv(  //
      "./main",                        //
      "--help");
  auto argo = Argo::Parser<"Help flag">("program");
  auto parser = argo.addArg<"arg0,k", int>()
                    .addArg<"arg1,a", int, Argo::nargs('+')>()
                    .addHelp();

  testing::internal::CaptureStdout();
  EXPECT_EXIT(parser.parse(argc, argv), testing::ExitedWithCode(0), "");
  testing::internal::GetCapturedStdout();
}

TEST(ArgoTest, ShortHelpFlag) {
  auto [argc, argv] = createArgcArgv(  //
      "./main",                        //
      "-h");
  auto argo = Argo::Parser<"Help short flag">("program");
  auto parser = argo.addArg<"arg0,k", int>()
                    .addArg<"arg1,a", int, Argo::nargs('+')>()
                    .addHelp();

  testing::internal::CaptureStdout();
  EXPECT_EXIT(parser.parse(argc, argv), testing::ExitedWithCode(0), "");
  testing::internal::GetCapturedStdout();
}

TEST(ArgoTest, ChangeHelpFlag) {
  auto [argc, argv] = createArgcArgv(  //
      "./main",                        //
      "-g");
  auto argo = Argo::Parser<"Help Change Flag">("program");
  auto parser = argo.addArg<"arg0,k", int>()
                    .addArg<"arg1,a", int, Argo::nargs('+')>()
                    .addHelp<"generate_help,g">();

  testing::internal::CaptureStdout();
  EXPECT_EXIT(parser.parse(argc, argv), testing::ExitedWithCode(0), "");
  testing::internal::GetCapturedStdout();
}
