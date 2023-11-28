import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

using Argo::key;

TEST(ArgoTest, ExampleCode) {
  auto [argc, argv] = createArgcArgv("./main", "--arg1", "42", "--arg3", "Hello,World");

  auto parser = Argo::Parser<key("ExampleCode")>("Program name")  //
                    .addArg<int, key("arg1")>()
                    .addArg<float, key("arg2")>(Argo::explicitDefault(12.34))
                    .addArg<std::string, key("arg3")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<key("arg1")>(), 42);
  EXPECT_FLOAT_EQ(parser.getArg<key("arg2")>(), 12.34);
  EXPECT_EQ(parser.getArg<key("arg3")>(), "Hello,World");

  EXPECT_TRUE((std::is_same_v<decltype(parser.getArg<key("arg1")>()), int>));
  EXPECT_TRUE((std::is_same_v<decltype(parser.getArg<key("arg2")>()), float>));
  EXPECT_TRUE((std::is_same_v<decltype(parser.getArg<key("arg3")>()), std::string>));
}
