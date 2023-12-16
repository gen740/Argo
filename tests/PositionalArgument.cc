import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

TEST(ArgoTest, Positional) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "1", "2", "3"                    //
    );

    auto argo = Argo::Parser<"Positional arguments">("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<"arg1", int, Argo::nargs(3)>();

    parser.parse(argc, argv);
    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "-a", "1", "2", "3",   //
        "--arg2", "42.195"               //
    );

    auto argo = Argo::Parser<"positional argument2">("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<"arg1", int, Argo::nargs(3)>()
                      .addArg<"arg2", float>()
                      .addFlag<"arg3,a">();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_FLOAT_EQ(parser.getArg<"arg2">(), 42.195);
    EXPECT_EQ(parser.getArg<"arg3">(), true);
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg2", "42.195",    //
        "1", "2", "3"                    //
    );

    auto argo = Argo::Parser<"positional argument3">("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<"arg1", int, Argo::nargs(3)>()
                      .addArg<"arg2", float, Argo::nargs(1)>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_FLOAT_EQ(parser.getArg<"arg2">(), 42.195);
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg2", "42", "96",  //
        "1", "2", "3"                    //
    );

    auto argo = Argo::Parser<"positonal argument 4">("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<"arg1", int, Argo::nargs(3)>()
                      .addArg<"arg2", float, Argo::nargs(2)>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_THAT(parser.getArg<"arg2">(), testing::ElementsAre(42, 96));
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg2",              //
        "1", "2", "3"                    //
    );

    auto argo = Argo::Parser<"positonal argument 5">("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<"arg1", int, Argo::nargs(3)>()
                      .addFlag<"arg2">();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_TRUE(parser.getArg<"arg2">());
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "1", "2", "3", "-b"              //
    );

    auto argo = Argo::Parser<"positonal argument 6">("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<"arg1", int, Argo::nargs(3)>()
                      .addFlag<"arg2,b">();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_TRUE(parser.getArg<"arg2">());
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "1", "2", "3", "-bc", "234.86"   //
    );

    auto argo = Argo::Parser<"positonal argument 7">("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<"arg1", int, Argo::nargs(3)>()
                      .addFlag<"arg2,b">()
                      .addArg<"arg3,c", float>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_TRUE(parser.getArg<"arg2">());
    EXPECT_FLOAT_EQ(parser.getArg<"arg3">(), 234.86);
  }
}

TEST(ArgoTest, MultiplePositional) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "1", "2", "3",                   //
        "4", "5", "6",                   //
        "-c", "234.86"                   //
    );

    auto argo = Argo::Parser<"multiple positonal argument 0">("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<"arg1", int, Argo::nargs(3)>()
                      .addPositionalArg<"arg2", int, Argo::nargs(3)>()
                      .addArg<"arg3,c", float>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_THAT(parser.getArg<"arg2">(), testing::ElementsAre(4, 5, 6));
  }

  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "1", "2", "3",                   //
        "-c", "234.86",                  //
        "4", "5", "6"                    //
    );

    auto argo = Argo::Parser<"multiple positonal argument 1">("Sample Program");
    auto parser =
        argo  //
            .addPositionalArg<"arg1", std::array<int, 3>, Argo::nargs(3)>()
            .addArg<"arg3,c", float>()
            .addPositionalArg<"arg2", int, Argo::nargs(3)>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_THAT(parser.getArg<"arg2">(), testing::ElementsAre(4, 5, 6));
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "1", "2", "3",                   //
        "-c", "234.86",                  //
        "4.2", "5", "Hello,World"        //
    );

    auto argo = Argo::Parser<"multiple positonal argument 2">("Sample Program");
    auto parser =
        argo  //
            .addPositionalArg<"arg1", std::array<int, 3>, Argo::nargs(3)>()
            .addArg<"arg3,c", float>()
            .addPositionalArg<"arg2", std::tuple<float, int, std::string>>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_FLOAT_EQ(std::get<0>(parser.getArg<"arg2">()), 4.2);
    EXPECT_EQ(std::get<1>(parser.getArg<"arg2">()), 5);
    EXPECT_EQ(std::get<2>(parser.getArg<"arg2">()), "Hello,World");
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "1", "2", "3",                   //
        "-c", "234.86",                  //
        "4.2", "5", "Hello,World"        //
    );

    auto argo = Argo::Parser<"multiple positonal argument 3">("Sample Program");
    auto parser =
        argo  //
            .addPositionalArg<"arg1", int, Argo::nargs('+')>()
            .addPositionalArg<"arg2", std::tuple<float, int, std::string>>()
            .addArg<"arg3,c", float>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_FLOAT_EQ(std::get<0>(parser.getArg<"arg2">()), 4.2);
    EXPECT_EQ(std::get<1>(parser.getArg<"arg2">()), 5);
    EXPECT_EQ(std::get<2>(parser.getArg<"arg2">()), "Hello,World");
  }
}
