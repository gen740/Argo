import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

using Argo::implicitDefault;
using Argo::InvalidArgument;
using Argo::nargs;
using Argo::Parser;
using Argo::ValidationError;
using Argo::Validation::Range;

TEST(ArgoTest, EqualAssign) {
  auto [argc, argv] =
      createArgcArgv("./main", "--arg1=42", "--arg2=Hello,World");

  auto argo = Parser<"Equal assign">();
  auto parser = argo                        //
                    .addArg<"arg1", int>()  //
                    .addArg<"arg2", std::string>();

  parser.parse(argc, argv.get());

  EXPECT_EQ(parser.getArg<"arg1">(), 42);
  EXPECT_EQ(parser.getArg<"arg2">(), "Hello,World");
}

TEST(ArgoTest, FlagArgument) {
  auto [argc, argv] = createArgcArgv(  //
      "./main",                        //
      "--arg1", "--arg2=true",
      "--arg3=1",         //
      "--arg4", "true",   //
      "--arg5", "false",  //
      "--arg6", "1"       //
  );

  auto argo = Parser<"Flag arguments">();
  auto parser = argo.addFlag<"arg1">()
                    .addArg<"arg2", bool>()
                    .addArg<"arg3", bool>()
                    .addArg<"arg4", bool>()
                    .addArg<"arg5", bool>()
                    .addArg<"arg6", bool>();

  parser.parse(argc, argv.get());

  EXPECT_EQ(parser.getArg<"arg1">(), true);
  EXPECT_EQ(parser.getArg<"arg2">(), true);
  EXPECT_EQ(parser.getArg<"arg3">(), true);
  EXPECT_EQ(parser.getArg<"arg4">(), true);
  EXPECT_EQ(parser.getArg<"arg5">(), false);
  EXPECT_EQ(parser.getArg<"arg6">(), true);
}

TEST(ArgoTest, ShortArgument) {
  auto [argc, argv] = createArgcArgv(  //
      "./main",                        //
      "-a",
      "Hello,World",  //
      "--arg2",
      "42",  //
      "-c",
      "3.1415"  //
  );

  auto argo = Parser<"Short argument">();
  auto parser = argo.addArg<"arg1,a", std::string>()  //
                    .addArg<"arg2,b", int>()
                    .addArg<"arg3,c", float>();

  parser.parse(argc, argv.get());

  EXPECT_EQ(parser.getArg<"arg1">(), "Hello,World");
  EXPECT_EQ(parser.getArg<"arg2">(), 42);
  EXPECT_FLOAT_EQ(parser.getArg<"arg3">(), 3.1415);
}

TEST(ArgoTest, CombiningFlags) {
  auto [argc, argv] = createArgcArgv(  //
      "./main", "-abd",
      "-e"  //
  );

  auto argo = Parser<"Combined flags">();
  auto parser = argo.addFlag<"arg1,a">()
                    .addFlag<"arg2,b">()
                    .addFlag<"arg3,c">()
                    .addFlag<"arg4,d">()
                    .addFlag<"arg5,e">();

  parser.parse(argc, argv.get());

  EXPECT_TRUE(parser.getArg<"arg1">());
  EXPECT_TRUE(parser.getArg<"arg2">());
  EXPECT_FALSE(parser.getArg<"arg3">());
  EXPECT_TRUE(parser.getArg<"arg4">());
  EXPECT_TRUE(parser.getArg<"arg5">());
}

TEST(ArgoTest, CombiningFlagsWithOptionalArg) {
  auto [argc, argv] = createArgcArgv(  //
      "./main", "-abdc",
      "Hello,World"  //
  );

  auto argo = Parser<"flag with optional args">();
  auto parser = argo.addFlag<"arg1,a">()
                    .addFlag<"arg2,b">()
                    .addArg<"arg3,c", std::string>()
                    .addFlag<"arg4,d">();

  parser.parse(argc, argv.get());

  EXPECT_TRUE(parser.getArg<"arg1">());
  EXPECT_TRUE(parser.getArg<"arg2">());
  EXPECT_EQ(parser.getArg<"arg3">(), "Hello,World");
  EXPECT_TRUE(parser.getArg<"arg4">());
}

TEST(ArgoTest, Validation) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg",
        "42"  //
    );

    auto argo = Parser<"Validation 1">();

    auto parser = argo  //
                      .addArg<"arg", int>(Range(0, 100))
                      .addFlag<"arg2">();

    parser.parse(argc, argv.get());
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg",
        "121"  //
    );

    auto argo = Parser<"Validation 2">();
    auto parser = argo  //
                      .addArg<"arg", int>(Range(0, 100))
                      .addFlag<"arg2">();

    EXPECT_THROW(parser.parse(argc, argv.get()), ValidationError);
  }
  //   {
  //     auto [argc, argv] = createArgcArgv(  //
  //         "./main", "--arg", "41"          //
  //     );
  //
  //     auto argo = Parser<"Validation 3">();
  //     auto parser = argo  //
  //                       .addArg<"arg", int>(new
  //                       Argo::Validation::Callback<int>(
  //                           [](auto value) { return value % 2 == 0; }))
  //                       .addFlag<"arg2">();
  //     EXPECT_THROW(parser.parse(argc, argv), ValidationError);
  //   }
  //
  //   {
  //     auto [argc, argv] = createArgcArgv(  //
  //         "./main", "--arg", "42"          //
  //     );
  //
  //     auto argo = Parser<"Validation 4">();
  //     auto parser = argo  //
  //                       .addArg<"arg", int>(new
  //                       Argo::Validation::Callback<int>(
  //                           [](auto value) { return value % 2 == 0; }))
  //                       .addFlag<"arg2">();
  //     parser.parse(argc, argv);
  //   }
}

TEST(ArgoTest, Narg) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1", "1", "2",
        "3",       //
        "--arg2",  //
        "--arg3", "6.0", "7.2", "8.4",
        "9.6",  //
        "--arg4", "11", "12", "8",
        "9"  //
    );

    auto argo = Parser<"Narg 1">();

    auto parser = argo  //
                      .addArg<"arg1", int, nargs(3)>()
                      .addArg<"arg2", std::string>(implicitDefault("Bar"))
                      .addArg<"arg3", float, nargs('*')>()
                      .addArg<"arg4", float, nargs('+')>();

    parser.parse(argc, argv.get());

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_EQ(parser.getArg<"arg2">(), "Bar");
    EXPECT_THAT(parser.getArg<"arg3">(),
                testing::ElementsAre(6.0, 7.2, 8.4, 9.6));
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1",
        "1"  //
    );
    auto argo = Parser<"Narg 2">();

    auto parser = argo  //
                      .addArg<"arg1", int, nargs(1)>();

    parser.parse(argc, argv.get());
  }
}

TEST(ArgoTest, NargException) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1"                         //
    );

    auto argo = Parser<"Narg exception">();

    auto parser = argo  //
                      .addArg<"arg1", int, nargs('+')>();

    EXPECT_THROW(parser.parse(argc, argv.get()), InvalidArgument);
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1",
        "--arg2"  //
    );

    auto argo = Parser<"Narg exception 2">();

    auto parser = argo  //
                      .addArg<"arg1", int, nargs('+')>()
                      .addArg<"arg2", int>();

    EXPECT_THROW(parser.parse(argc, argv.get()), InvalidArgument);
  }
}

TEST(ArgoTest, Required) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1", "1", "--arg2",
        "2"  //
    );

    auto argo = Parser<"Required argument">("Sample Program");
    auto parser = argo  //
                      .addArg<"arg1,a", int, nargs(1)>()
                      .addArg<"arg2", int, Argo::Required, nargs(1)>()
                      .addArg<"arg3", int, nargs(1), Argo::Required>()
                      .addArg<"arg4,b", int, nargs(1)>()
                      .addArg<"arg5,c", int, nargs(1)>();

    EXPECT_THROW(parser.parse(argc, argv.get()), InvalidArgument);
  }
}

TEST(ArgoTest, IsAssigned) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1", "42", "--arg2",
        "--arg4"  //
    );

    auto argo = Parser<"Is assigned">("Sample Program");
    auto parser = argo                                  //
                      .addArg<"arg1", int, nargs(1)>()  //
                      .addFlag<"arg2">()                //
                      .addArg<"arg3", int, nargs(1)>()  //
                      .addFlag<"arg4">();

    parser.parse(argc, argv.get());

    EXPECT_TRUE(parser.isAssigned<"arg1">());
    EXPECT_TRUE(parser.isAssigned<"arg2">());
    EXPECT_FALSE(parser.isAssigned<"arg3">());
    EXPECT_TRUE(parser.isAssigned<"arg4">());
  }
}

TEST(ArgoTest, CallBack) {
  {
    auto [argc, argv] = createArgcArgv("./main", "1", "2", "3");

    auto argo = Parser<150>("Sample Program");
    auto parser = argo.addPositionalArg<"arg1", int, nargs(3)>(
        [](std::array<int, 3>& value, std::span<std::string_view> raw) {
          EXPECT_THAT(value, testing::ElementsAre(1, 2, 3));
          EXPECT_EQ(raw[0], "1");
          EXPECT_EQ(raw[1], "2");
          EXPECT_EQ(raw[2], "3");
          for (auto& i : value) {
            i *= 2;
            i += 5;
          }
        });
    parser.parse(argc, argv.get());
    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(7, 9, 11));
  }
}
