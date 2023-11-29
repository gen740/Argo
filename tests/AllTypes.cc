import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

using Argo::key;

TEST(ArgoTest, AllTypes) {
  auto [argc, argv] = createArgcArgv(       //
      "./main",                             //
      "--arg1", "42",                       // int
      "--arg2", "42.1234567890",            // float
      "--arg3", "42.12345678901234567890",  // double
      "--arg4", "Hello,World!",             // string
      "--arg5", "Hello,World!const char*"   // const char*
  );

  auto argo = Argo::Parser<10>();
  auto parser = argo.addArg<key("arg1"), int>()
                    .addArg<key("arg2"), float>()
                    .addArg<key("arg3"), double>()
                    .addArg<key("arg4"), std::string>()
                    .addArg<key("arg5"), const char*>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<key("arg1")>(), 42);
  EXPECT_FLOAT_EQ(parser.getArg<key("arg2")>(), 42.1234567890f);
  EXPECT_DOUBLE_EQ(parser.getArg<key("arg3")>(), 42.12345678901234567890);
  EXPECT_EQ(parser.getArg<key("arg4")>(), "Hello,World!");
  EXPECT_TRUE(std::strcmp(parser.getArg<key("arg5")>(), "Hello,World!const char*") == 0);

  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<key("arg1")>()), int>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<key("arg2")>()), float>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<key("arg3")>()), double>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<key("arg4")>()), std::string>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<key("arg5")>()), const char*>));
}
