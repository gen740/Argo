import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

using testing::HasSubstr;
using testing::Property;
using testing::Throws;
using testing::ThrowsMessage;

using namespace Argo;

TEST(ArgoTest, ExceptionParseError) {
  auto [argc, argv] = createArgcArgv("./main", "foo");

  auto argo = Parser<"ExceptionParseError">();
  auto parser = argo.addArg<"arg1", int>()  //
                    .addArg<"arg2", float>();

  EXPECT_THAT([&parser]() { parser.getArg<"arg1">(); },
              ThrowsMessage<ParseError>(HasSubstr(
                  "Parser did not parse argument, call parse first")));
  EXPECT_THAT([&parser]() { parser.isAssigned<"arg1">(); },
              ThrowsMessage<ParseError>(HasSubstr(
                  "Parser did not parse argument, call parse first")));
  EXPECT_THAT([&]() { parser.parse(argc, argv); },
              ThrowsMessage<InvalidArgument>(
                  HasSubstr("Invalid positional argument: [\"foo\"]")));
}
