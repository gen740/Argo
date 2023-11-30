import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "TestHelper.h"

using Argo::InvalidArgument;

TEST(ArgoTest, ExceptionThow) {
  auto [argc, argv] = createArgcArgv("./main", "arg1=42", "--arg2=23.4");

  auto argo = Argo::Parser<20>();
  auto parser = argo.addArg<"arg1", int>()  //
                    .addArg<"arg2", float>();

  EXPECT_THROW(parser.getArg<"arg1">(), Argo::ParseError);
  EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
}

TEST(ArgoTest, EqualAssign) {
  auto [argc, argv] = createArgcArgv("./main", "--arg1=42", "--arg2=Hello,World");

  auto argo = Argo::Parser<30>();
  auto parser = argo                        //
                    .addArg<"arg1", int>()  //
                    .addArg<"arg2", std::string>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<"arg1">(), 42);
  EXPECT_EQ(parser.getArg<"arg2">(), "Hello,World");
}

TEST(ArgoTest, FlagArgument) {
  auto [argc, argv] = createArgcArgv(       //
      "./main",                             //
      "--arg1", "--arg2=true", "--arg3=1",  //
      "--arg4", "true", "--arg5", "1"       //
  );

  auto argo = Argo::Parser<40>();
  auto parser = argo.addFlag<"arg1">()
                    .addArg<"arg2", bool>()
                    .addArg<"arg3", bool>()
                    .addArg<"arg4", bool>()
                    .addArg<"arg5", bool>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<"arg1">(), true);
  EXPECT_EQ(parser.getArg<"arg2">(), true);
  EXPECT_EQ(parser.getArg<"arg3">(), true);
  EXPECT_EQ(parser.getArg<"arg4">(), true);
  EXPECT_EQ(parser.getArg<"arg5">(), true);
}

TEST(ArgoTest, ShortArgument) {
  auto [argc, argv] = createArgcArgv(  //
      "./main",                        //
      "-a", "Hello,World",             //
      "--arg2", "42",                  //
      "-c", "3.1415"                   //
  );

  auto argo = Argo::Parser<50>();
  auto parser = argo.addArg<"arg1,a", std::string>()  //
                    .addArg<"arg2,b", int>()
                    .addArg<"arg3,c", float>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<"arg1">(), "Hello,World");
  EXPECT_EQ(parser.getArg<"arg2">(), 42);
  EXPECT_FLOAT_EQ(parser.getArg<"arg3">(), 3.1415);
}

TEST(ArgoTest, CombiningFlags) {
  auto [argc, argv] = createArgcArgv(  //
      "./main", "-abd", "-e"           //
  );

  auto argo = Argo::Parser<60>();
  auto parser = argo.addFlag<"arg1,a">()
                    .addFlag<"arg2,b">()
                    .addFlag<"arg3,c">()
                    .addFlag<"arg4,d">()
                    .addFlag<"arg5,e">();

  parser.parse(argc, argv);

  EXPECT_TRUE(parser.getArg<"arg1">());
  EXPECT_TRUE(parser.getArg<"arg2">());
  EXPECT_FALSE(parser.getArg<"arg3">());
  EXPECT_TRUE(parser.getArg<"arg4">());
  EXPECT_TRUE(parser.getArg<"arg5">());
}

TEST(ArgoTest, CombiningFlagsWithOptionalArg) {
  auto [argc, argv] = createArgcArgv(   //
      "./main", "-abdc", "Hello,World"  //
  );

  auto argo = Argo::Parser<70>();
  auto parser = argo.addFlag<"arg1,a">()
                    .addFlag<"arg2,b">()
                    .addArg<"arg3,c", std::string>()
                    .addFlag<"arg4,d">();

  parser.parse(argc, argv);

  EXPECT_TRUE(parser.getArg<"arg1">());
  EXPECT_TRUE(parser.getArg<"arg2">());
  EXPECT_EQ(parser.getArg<"arg3">(), "Hello,World");
  EXPECT_TRUE(parser.getArg<"arg4">());
}

TEST(ArgoTest, Validation) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg", "42"          //
    );

    auto argo = Argo::Parser<80>();

    auto parser = argo  //
                      .addArg<"arg", int>(new Argo::Validation::MinMax<int>(0, 100))
                      .addFlag<"arg2">();

    parser.parse(argc, argv);
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg", "121"         //
    );

    auto argo = Argo::Parser<81>();
    auto parser = argo  //
                      .addArg<"arg", int>(new Argo::Validation::MinMax<int>(0, 100))
                      .addFlag<"arg2">();

    EXPECT_THROW(parser.parse(argc, argv), Argo::ValidationError);
  }
}

TEST(ArgoTest, Narg) {
  {
    auto [argc, argv] = createArgcArgv(        //
        "./main",                              //
        "--arg1", "1", "2", "3",               //
        "--arg2",                              //
        "--arg3", "6.0", "7.2", "8.4", "9.6",  //
        "--arg4", "11", "12", "8", "9"         //
    );

    auto argo = Argo::Parser<90>();

    auto parser = argo  //
                      .addArg<"arg1", int, Argo::nargs(3)>()
                      .addArg<"arg2", std::string>(Argo::implicitDefault("Bar"))
                      .addArg<"arg3", float, Argo::nargs('*')>()
                      .addArg<"arg4", float, Argo::nargs('+')>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_EQ(parser.getArg<"arg2">(), "Bar");
    EXPECT_THAT(parser.getArg<"arg3">(), testing::ElementsAre(6.0, 7.2, 8.4, 9.6));
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1", "1"                    //
    );
    auto argo = Argo::Parser<91>();

    auto parser = argo  //
                      .addArg<"arg1", int, Argo::nargs(1)>();

    parser.parse(argc, argv);
  }
}

TEST(ArgoTest, NargException) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1"                         //
    );

    auto argo = Argo::Parser<100>();

    auto parser = argo  //
                      .addArg<"arg1", int, Argo::nargs('+')>();

    EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
  }

  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1", "--arg2"               //
    );

    auto argo = Argo::Parser<101>();

    auto parser = argo  //
                      .addArg<"arg1", int, Argo::nargs('+')>()
                      .addArg<"arg2", int>();

    EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
  }
}

TEST(ArgoTest, Help) {  // TODO(gen740): more help
  {
    auto argo = Argo::Parser<110>("program");
    auto parser = argo  //
                      .addArg<"arg0,k", int>()
                      .addArg<"arg1,a", int, Argo::nargs('+')>()
                      .addArg<"arg2", int, Argo::nargs('+')>(Argo::description("This is arg2"));

    auto expect_help = R"(Options:
  -k, --arg0
  -a, --arg1
      --arg2  This is arg2)";

    EXPECT_EQ(parser.formatHelp(), expect_help);
  }
}

TEST(ArgoTest, Required) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1", "1", "--arg2", "2"     //
    );

    auto argo = Argo::Parser<120>("Sample Program");
    auto parser = argo  //
                      .addArg<"arg1,a", int, Argo::nargs(1)>()
                      .addArg<"arg2", int, Argo::required, Argo::nargs(1)>()
                      .addArg<"arg3", int, Argo::nargs(1), Argo::required>()
                      .addArg<"arg4,b", int, Argo::nargs(1)>()
                      .addArg<"arg5,c", int, Argo::nargs(1)>();

    EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
  }
}

TEST(ArgoTest, Positional) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "1", "2", "3"                    //
    );

    auto argo = Argo::Parser<130>("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<"arg1", int, Argo::nargs(3)>();

    parser.parse(argc, argv);
    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "1", "2", "3",         //
        "--arg2", "42.195"               //
    );

    auto argo = Argo::Parser<131>("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<"arg1", int, Argo::nargs(3)>()
                      .addArg<"arg2", float>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_FLOAT_EQ(parser.getArg<"arg2">(), 42.195);
  }

  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg2", "42.195",    //
        "1", "2", "3"                    //
    );

    auto argo = Argo::Parser<132>("Sample Program");
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

    auto argo = Argo::Parser<133>("Sample Program");
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

    auto argo = Argo::Parser<134>("Sample Program");
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

    auto argo = Argo::Parser<135>("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<"arg1", int, Argo::nargs(3)>()
                      .addFlag<"arg2,b">();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
    EXPECT_TRUE(parser.getArg<"arg2">());
  }

  {                                      // error
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "1", "2", "3", "-bc", "234.86"   //
    );

    auto argo = Argo::Parser<136>("Sample Program");
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

// TEST(ArgoTest, IsAssigned) {
//   {
//     const int argc = 4;
//     char* argv[argc] = {
//         "./main",       //
//         "1", "2", "3",  //
//     };
//
//     auto argo = Argo::Parser<130>("Sample Program");
//     auto parser = argo  //
//                       .addPositionalArg<int, key("arg1"), 'a', Argo::nargs(3)>();
//
//     parser.parse(argc, argv);
//     EXPECT_THAT(parser.getArg<key("arg1")>(), testing::ElementsAre(1, 2, 3));
//   }
// }

TEST(ArgoTest, CallBack) {
  {
    const int argc = 4;
    char* argv[argc] = {
        "./main",       //
        "1", "2", "3",  //
    };

    auto argo = Argo::Parser<150>("Sample Program");
    auto parser = argo.addPositionalArg<"arg1", int, Argo::nargs(3)>(
        [](auto value, [[maybe_unused]] auto raw) {
          EXPECT_THAT(value, testing::ElementsAre(1, 2, 3));
          // EXPECT_EQ(key, "arg1");
        });

    parser.parse(argc, argv);
    EXPECT_THAT(parser.getArg<"arg1">(), testing::ElementsAre(1, 2, 3));
  }
}
