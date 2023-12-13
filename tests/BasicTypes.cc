import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

TEST(ArgoTest, AllTypes) {
  auto [argc, argv] = createArgcArgv(       //
      "./main",                             //
      "--arg1", "42",                       // int
      "--arg2", "-42",                      // nagitive int
      "--arg3", "42.1234567890",            // float
      "--arg4", "42.12345678901234567890",  // double
      "--arg5", "Hello,World!",             // string
      "--arg5", "Hello,World!const char*"   // const char*
  );

  auto argo = Argo::Parser<10>();
  auto parser = argo.addArg<"arg1", int>()
                    .addArg<"arg2", int>()
                    .addArg<"arg3", float>()
                    .addArg<"arg4", double>()
                    .addArg<"arg5", std::string>()
                    .addArg<"arg6", const char*>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<"arg1">(), 42);
  EXPECT_EQ(parser.getArg<"arg2">(), -42);
  EXPECT_FLOAT_EQ(parser.getArg<"arg3">(), 42.1234567890f);
  EXPECT_DOUBLE_EQ(parser.getArg<"arg4">(), 42.12345678901234567890);
  EXPECT_EQ(parser.getArg<"arg5">(), "Hello,World!");
  EXPECT_TRUE(std::strcmp(parser.getArg<"arg6">(), "Hello,World!const char*") ==
              0);

  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<"arg1">()), int>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<"arg2">()), int>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<"arg3">()), float>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<"arg4">()), double>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<"arg5">()), std::string>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<"arg6">()), const char*>));
}
