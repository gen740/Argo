import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

using Argo::nargs;
using Argo::Parser;

TEST(ArgoTest, STLTypes) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1", "42", "43", "44"       // int
    );
    auto argo = Parser<"STLTypes_array">();
    auto parser = argo.addArg<"arg1", std::array<int, 3>>();
    parser.parse(argc, argv.get());
    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(42, 43, 44));
  }
  {
    auto [argc, argv] = createArgcArgv(   //
        "./main",                         //
        "--arg1", "42", "43", "44", "45"  // int
    );
    auto argo = Parser<"STLTypes_vector">();
    auto parser = argo.addArg<"arg1", std::vector<int>, nargs(4)>();
    parser.parse(argc, argv.get());
    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(42, 43, 44, 45));
  }

  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1", "42", "43.24",         //
        "Hello,World"                    //
    );
    auto argo = Parser<"STLTypes_tuple">();
    auto parser = argo.addArg<"arg1", std::tuple<int, double, std::string>>();
    parser.parse(argc, argv.get());

    auto [a1, a2, a3] = parser.getArg<"arg1">();
    EXPECT_EQ(a1, 42);
    EXPECT_DOUBLE_EQ(a2, 43.24);
    EXPECT_EQ(a3, "Hello,World");
  }
}
