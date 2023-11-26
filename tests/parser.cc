import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <print>
#include <string>
#include <type_traits>

using Argo::InvalidArgument;

TEST(ArgoTest, ExampleCode) {
  const int argc = 5;
  char* argv[argc] = {"./main", "--arg1", "42", "--arg3", "Hello,World"};

  auto argo = Argo::Parser();
  auto parser = argo.addArg<int, Argo::arg("arg1")>()
                    .addArg<float, Argo::arg("arg2")>()
                    .addArg<std::string, Argo::arg("arg3")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::arg("arg1")>().value(), 42);
  EXPECT_FALSE(parser.getArg<Argo::arg("arg2")>().has_value());
  EXPECT_EQ(parser.getArg<Argo::arg("arg3")>().value(), "Hello,World");

  EXPECT_TRUE((                                                     //
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg1")>()),  //
                     std::optional<int>>));
  EXPECT_TRUE((                                                     //
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg2")>()),  //
                     std::optional<float>>));
  EXPECT_TRUE((                                                     //
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg3")>()),  //
                     std::optional<std::string>>));
}

TEST(ArgoTest, AllTypes) {
  const int argc = 11;
  char* argv[argc] = {
      "./main",                             //
      "--arg1", "42",                       // int
      "--arg2", "42.1234567890",            // float
      "--arg3", "42.12345678901234567890",  // double
      "--arg4", "Hello,World!",             // string
      "--arg5", "Hello,World!const char*",  // const char*
  };

  auto argo = Argo::Parser<10>();
  auto parser = argo.addArg<int, Argo::arg("arg1")>()
                    .addArg<float, Argo::arg("arg2")>()
                    .addArg<double, Argo::arg("arg3")>()
                    .addArg<std::string, Argo::arg("arg4")>()
                    .addArg<const char*, Argo::arg("arg5")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::arg("arg1")>().value(), 42);
  EXPECT_FLOAT_EQ(parser.getArg<Argo::arg("arg2")>().value(), 42.1234567890f);
  EXPECT_DOUBLE_EQ(parser.getArg<Argo::arg("arg3")>().value(), 42.12345678901234567890);
  EXPECT_EQ(parser.getArg<Argo::arg("arg4")>().value(), "Hello,World!");
  EXPECT_EQ(parser.getArg<Argo::arg("arg5")>().value(), "Hello,World!const char*");

  EXPECT_TRUE((                                                     //
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg1")>()),  //
                     std::optional<int>>));
  EXPECT_TRUE((                                                     //
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg2")>()),  //
                     std::optional<float>>));
  EXPECT_TRUE((                                                     //
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg3")>()),  //
                     std::optional<double>>));
  EXPECT_TRUE((                                                     //
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg4")>()),  //
                     std::optional<std::string>>));
  EXPECT_TRUE((                                                     //
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg5")>()),  //
                     std::optional<const char*>>));
}

TEST(ArgoTest, ExceptionThow) {
  const int argc = 3;
  char* argv[argc] = {"./main", "arg1=42", "--arg2=23.4"};

  auto argo = Argo::Parser<20>();
  auto parser = argo.addArg<int, Argo::arg("arg1")>()  //
                    .addArg<float, Argo::arg("arg2")>();

  EXPECT_THROW(parser.getArg<Argo::arg("arg1")>().value(), Argo::ParseError);
  EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
}

TEST(ArgoTest, EqualAssign) {
  const int argc = 3;
  char* argv[argc] = {"./main", "--arg1=42", "--arg2=Hello,World"};

  auto argo = Argo::Parser<30>();
  auto parser = argo.addArg<int, Argo::arg("arg1")>().addArg<std::string, Argo::arg("arg2")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::arg("arg1")>().value(), 42);
  EXPECT_EQ(parser.getArg<Argo::arg("arg2")>().value(), "Hello,World");
}

TEST(ArgoTest, FlagArgument) {
  const int argc = 8;
  char* argv[argc] = {"./main", "--arg1", "--arg2=true", "--arg3=1",
                      "--arg4", "true",   "--arg5",      "1"};

  auto argo = Argo::Parser<40>();
  auto parser = argo.addFlag<Argo::arg("arg1")>()
                    .addArg<bool, Argo::arg("arg2")>()
                    .addArg<bool, Argo::arg("arg3")>()
                    .addArg<bool, Argo::arg("arg4")>()
                    .addArg<bool, Argo::arg("arg5")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::arg("arg1")>(), true);
  EXPECT_EQ(parser.getArg<Argo::arg("arg2")>().value(), true);
  EXPECT_EQ(parser.getArg<Argo::arg("arg3")>().value(), true);
  EXPECT_EQ(parser.getArg<Argo::arg("arg4")>().value(), true);
  EXPECT_EQ(parser.getArg<Argo::arg("arg5")>().value(), true);
}

TEST(ArgoTest, ShortArgument) {
  const int argc = 7;
  char* argv[argc] = {
      "./main",                 //
      "-a",     "Hello,World",  //
      "-b",     "42",           //
      "-c",     "3.1415"        //
  };

  auto argo = Argo::Parser<50>();
  auto parser = argo.addArg<std::string, Argo::arg("arg1"), 'a'>()
                    .addArg<int, Argo::arg("arg2"), 'b'>()
                    .addArg<float, Argo::arg("arg3"), 'c'>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::arg("arg1")>().value(), "Hello,World");
  EXPECT_EQ(parser.getArg<Argo::arg("arg2")>().value(), 42);
  EXPECT_FLOAT_EQ(parser.getArg<Argo::arg("arg3")>().value(), 3.1415);
}

TEST(ArgoTest, CombiningFlags) {
  const int argc = 3;
  char* argv[argc] = {
      "./main", "-abd", "-e"  //
  };

  auto argo = Argo::Parser<60>();
  auto parser = argo.addFlag<Argo::arg("arg1"), 'a'>()
                    .addFlag<Argo::arg("arg2"), 'b'>()
                    .addFlag<Argo::arg("arg3"), 'c'>()
                    .addFlag<Argo::arg("arg4"), 'd'>()
                    .addFlag<Argo::arg("arg5"), 'e'>();

  parser.parse(argc, argv);

  EXPECT_TRUE(parser.getArg<Argo::arg("arg1")>());
  EXPECT_TRUE(parser.getArg<Argo::arg("arg2")>());
  EXPECT_FALSE(parser.getArg<Argo::arg("arg3")>());
  EXPECT_TRUE(parser.getArg<Argo::arg("arg4")>());
  EXPECT_TRUE(parser.getArg<Argo::arg("arg5")>());
}

TEST(ArgoTest, CombiningFlagsWithOptionalArg) {
  const int argc = 3;
  char* argv[argc] = {
      "./main", "-abcd", "Hello,World"  //
  };

  auto argo = Argo::Parser<70>();
  auto parser = argo.addFlag<Argo::arg("arg1"), 'a'>()
                    .addFlag<Argo::arg("arg2"), 'b'>()
                    .addArg<std::string, Argo::arg("arg3"), 'c'>()
                    .addFlag<Argo::arg("arg4"), 'd'>();

  parser.parse(argc, argv);

  EXPECT_TRUE(parser.getArg<Argo::arg("arg1")>());
  EXPECT_TRUE(parser.getArg<Argo::arg("arg2")>());
  EXPECT_EQ(parser.getArg<Argo::arg("arg3")>().value(), "Hello,World");
  EXPECT_TRUE(parser.getArg<Argo::arg("arg4")>());
}

TEST(ArgoTest, Validation) {
  const int argc = 3;
  char* argv[argc] = {
      "./main", "--arg", "42"  //
  };

  auto argo = Argo::Parser<80>();

  auto parser = argo  //
                    .addArg<int, Argo::arg("arg")>(new Argo::Validation::MinMax<int>(0, 100))
                    .addFlag<Argo::arg("arg2")>();

  parser.parse(argc, argv);

  char* argv2[argc] = {
      "./main", "--arg", "121"  //
  };

  EXPECT_THROW(parser.parse(argc, argv2), Argo::ValidationError);
}

TEST(ArgoTest, Narg) {
  {
    const int argc = 16;
    char* argv[argc] = {
        //
        "./main",                              //
        "--arg1", "1",   "2",   "3",           //
        "--arg2",                              //
        "--arg3", "6.0", "7.2", "8.4", "9.6",  //
        "--arg4", "11",  "12",  "8",   "9"     //
    };

    auto argo = Argo::Parser<90>();

    auto parser = argo  //
                      .addArg<int, Argo::arg("arg1"), Argo::nargs(3)>()
                      .addArg<std::string, Argo::arg("arg2")>("Bar")
                      .addArg<float, Argo::arg("arg3"), Argo::nargs('*')>()
                      .addArg<float, Argo::arg("arg4"), Argo::nargs('+')>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<Argo::arg("arg1")>().value(), testing::ElementsAre(1, 2, 3));
    EXPECT_EQ(parser.getArg<Argo::arg("arg2")>().value(), "Bar");
    EXPECT_THAT(parser.getArg<Argo::arg("arg3")>().value(),
                testing::ElementsAre(6.0, 7.2, 8.4, 9.6));
  }
  {
    const int argc = 3;
    char* argv[argc] = {
        //
        "./main",      //
        "--arg1", "1"  //
    };

    auto argo = Argo::Parser<91>();

    auto parser = argo  //
                      .addArg<int, Argo::arg("arg1"), Argo::nargs(1)>();

    parser.parse(argc, argv);

    // EXPECT_THAT(parser.getArg<Argo::arg("arg1")>().value(), testing::ElementsAre(1, 2, 3));
  }
}

TEST(ArgoTest, NargException) {
  {
    const int argc = 2;
    char* argv[argc] = {
        "./main",  //
        "--arg1",  //
    };

    auto argo = Argo::Parser<100>();

    auto parser = argo  //
                      .addArg<int, Argo::arg("arg1"), Argo::nargs('+')>();

    EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
  }

  {
    const int argc = 3;
    char* argv[argc] = {
        "./main",            //
        "--arg1", "--arg2",  //
    };

    auto argo = Argo::Parser<101>();

    auto parser = argo  //
                      .addArg<int, Argo::arg("arg1"), Argo::nargs('+')>()
                      .addArg<int, Argo::arg("arg2")>();

    EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
  }
}

TEST(ArgoTest, Help) {
  {
    auto argo = Argo::Parser<110>("Sample Program");
    auto parser = argo  //
                      .addArg<int, Argo::arg("arg1"), 'a', Argo::nargs('+')>()
                      .addArg<int, Argo::arg("arg2"), Argo::nargs('+')>(
                          Argo::withDescription("This is arg2"));

    auto ArgInfo = parser.getArgInfo();

    EXPECT_EQ(ArgInfo[0].name, "arg1");
    EXPECT_EQ(ArgInfo[0].shortName, 'a');
    EXPECT_EQ(ArgInfo[0].description, "");

    EXPECT_EQ(ArgInfo[1].name, "arg2");
    EXPECT_EQ(ArgInfo[1].shortName, '\0');
    EXPECT_EQ(ArgInfo[1].description, "This is arg2");

    // EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
  }
  {
    auto argo = Argo::Parser<111>("program");
    auto parser = argo  //
                      .addArg<int, Argo::arg("arg1"), 'a', Argo::nargs('+')>()
                      .addArg<int, Argo::arg("arg2"), Argo::nargs('+')>(
                          Argo::withDescription("This is arg2"));

    auto ArgInfo = parser.getArgInfo();

    std::println(stderr, "{}", parser.formatHelp());

    // EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
  }
}
