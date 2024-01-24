import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <type_traits>

#include "TestHelper.h"

TEST(ArgoTest, ExampleCode) {
  auto [argc, argv] =
      createArgcArgv("./main", "--arg1", "42", "--arg3", "Hello,World");

  auto parser = Argo::Parser<0>("Program name")  //
                    .addArg<"arg1", int>()
                    .addArg<"arg2", float>(Argo::explicitDefault(12.34))
                    .addArg<"arg3", std::string>();

  parser.parse(argc, argv.get());

  EXPECT_EQ(parser.getArg<"arg1">(), 42);
  EXPECT_FLOAT_EQ(parser.getArg<"arg2">(), 12.34);
  EXPECT_EQ(parser.getArg<"arg3">(), "Hello,World");

  EXPECT_TRUE((std::is_same_v<decltype(parser.getArg<"arg1">()), int>));
  EXPECT_TRUE((std::is_same_v<decltype(parser.getArg<"arg2">()), float>));
  EXPECT_TRUE((std::is_same_v<decltype(parser.getArg<"arg3">()), std::string>));
};
