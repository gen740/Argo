import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

using Argo::Parser;

TEST(ArgoTest, SubCommands) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "cmd1", "--arg1", "42"           // int
    );

    auto parser1 = Parser<"SubCommands1_cmd1">()  //
                       .addArg<"arg1,a", int>()
                       .addArg<"arg2,b", int>(Argo::explicitDefault(123));
    auto parser2 = Parser<"SubCommands1_cmd2">()  //
                       .addArg<"arg3,a", int>()
                       .addArg<"arg4,b", int>();
    auto parser = Parser<"SubCommands1">()  //
                      .addParser<"cmd1">(parser1, Argo::description("cmd1"))
                      .addParser<"cmd2">(parser2, Argo::description("cmd2"));

    parser.parse(argc, argv.get());

    // same object
    EXPECT_EQ(&parser1, &parser.getParser<"cmd1">());
    EXPECT_EQ(&parser2, &parser.getParser<"cmd2">());

    EXPECT_EQ(parser1.getArg<"arg1">(), 42);
    EXPECT_EQ(parser1.getArg<"arg2">(), 123);

    EXPECT_TRUE(parser1);
    EXPECT_FALSE(parser2);
  }

  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "cmd1", "cmd3", "--arg1",
        "42"  // int
    );

    auto parser1 = Parser<"SubCommands2_cmd1">()  //
                       .addArg<"arg1,a", int>()
                       .addArg<"arg2,b", int>(Argo::explicitDefault(123));
    auto parser2 = Parser<"SubCommands2_cmd2">()  //
                       .addArg<"arg3,a", int>()
                       .addArg<"arg4,b", int>();
    auto parser3 = Parser<"SubCommands2_cmd3">()  //
                       .addArg<"arg5,a", int>()
                       .addArg<"arg6,b", int>()
                       .addParser<"cmd3">(parser1)
                       .addParser<"cmd4">(parser2);
    auto parser4 = Parser<"SubCommands2_cmd4">()  //
                       .addArg<"arg7,a", int>()
                       .addArg<"arg8,b", int>();
    auto parser = Parser<"SubCommands2">()  //
                      .addParser<"cmd1">(parser3, Argo::description("cmd1"))
                      .addParser<"cmd2">(parser4, Argo::description("cmd2"));

    parser.parse(argc, argv.get());

    // same object
    EXPECT_EQ(&parser1, &parser.getParser<"cmd1">().getParser<"cmd3">());
    EXPECT_EQ(&parser2, &parser.getParser<"cmd1">().getParser<"cmd4">());
    EXPECT_EQ(&parser3, &parser.getParser<"cmd1">());
    EXPECT_EQ(&parser4, &parser.getParser<"cmd2">());

    EXPECT_EQ(parser1.getArg<"arg1">(), 42);
    EXPECT_EQ(parser1.getArg<"arg2">(), 123);

    if (parser1) {
      EXPECT_TRUE(parser1);
    } else {
      EXPECT_TRUE(false);
    }

    EXPECT_TRUE(parser1);
    EXPECT_FALSE(parser2);
    EXPECT_TRUE(parser3);
    EXPECT_FALSE(parser4);
  }
}
