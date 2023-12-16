import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

using testing::HasSubstr;
using testing::Property;
using testing::Throws;
using testing::ThrowsMessage;

using namespace Argo;

auto parser1 = Parser<"ExceptionParseError">()
                   .addArg<"arg1", int>()
                   .addArg<"arg2", float>()
                   .addArg<"arg3", bool>()
                   .addArg<"arg4", int, nargs(3)>()
                   .addArg<"arg5", int, nargs(1)>();

TEST(ArgoTest, ExceptionParseError) {
  auto [argc, argv] = createArgcArgv("./main", "foo");

  EXPECT_THAT([]() { parser1.getArg<"arg1">(); },
              ThrowsMessage<ParseError>(HasSubstr(
                  "Parser did not parse argument, call parse first")));
  EXPECT_THAT([]() { parser1.isAssigned<"arg1">(); },
              ThrowsMessage<ParseError>(HasSubstr(
                  "Parser did not parse argument, call parse first")));
  EXPECT_THAT([&]() { parser1.parse(argc, argv); },
              ThrowsMessage<InvalidArgument>(
                  HasSubstr("Invalid positional argument: [\"foo\"]")));
  parser1.resetArgs();
}

TEST(ArgoTest, ExceptionOneArg) {
  auto [argc, argv] = createArgcArgv("./main", "--arg5");

  EXPECT_THAT([&]() { parser1.parse(argc, argv); },
              ThrowsMessage<InvalidArgument>(HasSubstr(
                  "Argument arg5: should take exactly one value but zero")));
  parser1.resetArgs();
}

auto parser2 = Parser<"Exception">()
                   .addArg<"arg1", int>()
                   .addArg<"arg2", float>()
                   .addArg<"arg3", bool>()
                   .addArg<"arg4", int, nargs(3)>()
                   .addPositionalArg<"arg5", int, nargs(4)>();

TEST(ArgoTest, ExceptionInvalidPositionalArg) {
  auto [argc, argv] = createArgcArgv("./main", "foo");
  EXPECT_THAT([&]() { parser2.parse(argc, argv); },
              ThrowsMessage<InvalidArgument>(
                  HasSubstr("Argument arg5: invalid argument [\"foo\"]")));
  parser2.resetArgs();
}

TEST(ArgoTest, ExceptionInvalidArgument) {
  auto [argc, argv] = createArgcArgv("./main", "--arg4", "1");
  EXPECT_THAT([&]() { parser2.parse(argc, argv); },
              ThrowsMessage<InvalidArgument>(
                  HasSubstr("Argument arg4: invalid argument [\"1\"]")));
}
