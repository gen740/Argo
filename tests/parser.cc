import Argo;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <print>
#include <string>
#include <type_traits>

template <typename... Args>
std::tuple<size_t, char**> createArgcArgv(Args... args) {
  const size_t N = sizeof...(Args);
  char** array = new char*[N];
  size_t i = 0;
  (..., (array[i++] = strdup(args)));
  return std::make_tuple(N, array);
}

using Argo::InvalidArgument;

TEST(ArgoTest, ExampleCode) {
  auto [argc, argv] = createArgcArgv("./main", "--arg1", "42", "--arg3", "Hello,World");
  auto argo = Argo::Parser();
  auto parser = argo.addArg<int, Argo::key("arg1")>()
                    .addArg<float, Argo::key("arg2")>()
                    .addArg<std::string, Argo::key("arg3")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::key("arg1")>(), 42);
  // EXPECT_FALSE(parser.getArg<Argo::key("arg2")>());
  EXPECT_EQ(parser.getArg<Argo::key("arg3")>(), "Hello,World");

  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<Argo::key("arg1")>()), int>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<Argo::key("arg2")>()), float>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<Argo::key("arg3")>()), std::string>));
}

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
  auto parser = argo.addArg<int, Argo::key("arg1")>()
                    .addArg<float, Argo::key("arg2")>()
                    .addArg<double, Argo::key("arg3")>()
                    .addArg<std::string, Argo::key("arg4")>()
                    .addArg<const char*, Argo::key("arg5")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::key("arg1")>(), 42);
  EXPECT_FLOAT_EQ(parser.getArg<Argo::key("arg2")>(), 42.1234567890f);
  EXPECT_DOUBLE_EQ(parser.getArg<Argo::key("arg3")>(), 42.12345678901234567890);
  EXPECT_EQ(parser.getArg<Argo::key("arg4")>(), "Hello,World!");
  EXPECT_TRUE(std::strcmp(parser.getArg<Argo::key("arg5")>(), "Hello,World!const char*") == 0);

  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<Argo::key("arg1")>()), int>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<Argo::key("arg2")>()), float>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<Argo::key("arg3")>()), double>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<Argo::key("arg4")>()), std::string>));
  EXPECT_TRUE((  //
      std::is_same_v<decltype(parser.getArg<Argo::key("arg5")>()), const char*>));
}

TEST(ArgoTest, ExceptionThow) {
  auto [argc, argv] = createArgcArgv("./main", "arg1=42", "--arg2=23.4");

  auto argo = Argo::Parser<20>();
  auto parser = argo.addArg<int, Argo::key("arg1")>()  //
                    .addArg<float, Argo::key("arg2")>();

  EXPECT_THROW(parser.getArg<Argo::key("arg1")>(), Argo::ParseError);
  EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
}

TEST(ArgoTest, EqualAssign) {
  auto [argc, argv] = createArgcArgv("./main", "--arg1=42", "--arg2=Hello,World");

  auto argo = Argo::Parser<30>();
  auto parser = argo                                   //
                    .addArg<int, Argo::key("arg1")>()  //
                    .addArg<std::string, Argo::key("arg2")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::key("arg1")>(), 42);
  EXPECT_EQ(parser.getArg<Argo::key("arg2")>(), "Hello,World");
}

TEST(ArgoTest, FlagArgument) {
  auto [argc, argv] = createArgcArgv(       //
      "./main",                             //
      "--arg1", "--arg2=true", "--arg3=1",  //
      "--arg4", "true", "--arg5", "1"       //
  );

  auto argo = Argo::Parser<40>();
  auto parser = argo.addFlag<Argo::key("arg1")>()
                    .addArg<bool, Argo::key("arg2")>()
                    .addArg<bool, Argo::key("arg3")>()
                    .addArg<bool, Argo::key("arg4")>()
                    .addArg<bool, Argo::key("arg5")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::key("arg1")>(), true);
  EXPECT_EQ(parser.getArg<Argo::key("arg2")>(), true);
  EXPECT_EQ(parser.getArg<Argo::key("arg3")>(), true);
  EXPECT_EQ(parser.getArg<Argo::key("arg4")>(), true);
  EXPECT_EQ(parser.getArg<Argo::key("arg5")>(), true);
}

TEST(ArgoTest, ShortArgument) {
  auto [argc, argv] = createArgcArgv(  //
      "./main",                        //
      "-a", "Hello,World",             //
      "-b", "42",                      //
      "-c", "3.1415"                   //
  );

  auto argo = Argo::Parser<50>();
  auto parser = argo.addArg<std::string, Argo::key("arg1"), 'a'>()
                    .addArg<int, Argo::key("arg2"), 'b'>()
                    .addArg<float, Argo::key("arg3"), 'c'>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::key("arg1")>(), "Hello,World");
  EXPECT_EQ(parser.getArg<Argo::key("arg2")>(), 42);
  EXPECT_FLOAT_EQ(parser.getArg<Argo::key("arg3")>(), 3.1415);
}

TEST(ArgoTest, CombiningFlags) {
  auto [argc, argv] = createArgcArgv(  //
      "./main", "-abd", "-e"           //
  );

  auto argo = Argo::Parser<60>();
  auto parser = argo.addFlag<Argo::key("arg1"), 'a'>()
                    .addFlag<Argo::key("arg2"), 'b'>()
                    .addFlag<Argo::key("arg3"), 'c'>()
                    .addFlag<Argo::key("arg4"), 'd'>()
                    .addFlag<Argo::key("arg5"), 'e'>();

  parser.parse(argc, argv);

  EXPECT_TRUE(parser.getArg<Argo::key("arg1")>());
  EXPECT_TRUE(parser.getArg<Argo::key("arg2")>());
  EXPECT_FALSE(parser.getArg<Argo::key("arg3")>());
  EXPECT_TRUE(parser.getArg<Argo::key("arg4")>());
  EXPECT_TRUE(parser.getArg<Argo::key("arg5")>());
}

TEST(ArgoTest, CombiningFlagsWithOptionalArg) {
  auto [argc, argv] = createArgcArgv(   //
      "./main", "-abdc", "Hello,World"  //
  );

  auto argo = Argo::Parser<70>();
  auto parser = argo.addFlag<Argo::key("arg1"), 'a'>()
                    .addFlag<Argo::key("arg2"), 'b'>()
                    .addArg<std::string, Argo::key("arg3"), 'c'>()
                    .addFlag<Argo::key("arg4"), 'd'>();

  parser.parse(argc, argv);

  EXPECT_TRUE(parser.getArg<Argo::key("arg1")>());
  EXPECT_TRUE(parser.getArg<Argo::key("arg2")>());
  EXPECT_EQ(parser.getArg<Argo::key("arg3")>(), "Hello,World");
  EXPECT_TRUE(parser.getArg<Argo::key("arg4")>());
}

TEST(ArgoTest, Validation) {
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg", "42"          //
    );

    auto argo = Argo::Parser<80>();

    auto parser = argo  //
                      .addArg<int, Argo::key("arg")>(new Argo::Validation::MinMax<int>(0, 100))
                      .addFlag<Argo::key("arg2")>();

    parser.parse(argc, argv);
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg", "121"         //
    );

    auto argo = Argo::Parser<81>();
    auto parser = argo  //
                      .addArg<int, Argo::key("arg")>(new Argo::Validation::MinMax<int>(0, 100))
                      .addFlag<Argo::key("arg2")>();

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
                      .addArg<int, Argo::key("arg1"), Argo::nargs(3)>()
                      .addArg<std::string, Argo::key("arg2")>(Argo::defaultValue("Bar"))
                      .addArg<float, Argo::key("arg3"), Argo::nargs('*')>()
                      .addArg<float, Argo::key("arg4"), Argo::nargs('+')>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<Argo::key("arg1")>(), testing::ElementsAre(1, 2, 3));
    EXPECT_EQ(parser.getArg<Argo::key("arg2")>(), "Bar");
    EXPECT_THAT(parser.getArg<Argo::key("arg3")>(), testing::ElementsAre(6.0, 7.2, 8.4, 9.6));
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1", "1"                    //
    );
    auto argo = Argo::Parser<91>();

    auto parser = argo  //
                      .addArg<int, Argo::key("arg1"), Argo::nargs(1)>();

    parser.parse(argc, argv);

    // EXPECT_THAT(parser.getArg<Argo::key("arg1")>(), testing::ElementsAre(1, 2, 3));
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
                      .addArg<int, Argo::key("arg1"), Argo::nargs('+')>();

    EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
  }

  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "--arg1", "--arg2"               //
    );

    auto argo = Argo::Parser<101>();

    auto parser = argo  //
                      .addArg<int, Argo::key("arg1"), Argo::nargs('+')>()
                      .addArg<int, Argo::key("arg2")>();

    EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
  }
}

TEST(ArgoTest, Help) {  // TODO(gen740): more help
  {
    auto argo = Argo::Parser<110>("Sample Program");
    auto parser = argo  //
                      .addArg<int, Argo::key("arg1"), 'a', Argo::nargs('+')>()
                      .addArg<int, Argo::key("arg2"), Argo::nargs('+')>(
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
                      .addArg<int, Argo::key("arg1"), 'a', Argo::nargs('+')>()
                      .addArg<int, Argo::key("arg2"), Argo::nargs('+')>(
                          Argo::withDescription("This is arg2"));

    auto ArgInfo = parser.getArgInfo();

    std::println(stderr, "{}", parser.formatHelp());

    // EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
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
                      .addArg<int, Argo::key("arg1"), 'a', Argo::nargs(1)>()
                      .addArg<int, Argo::key("arg2"), Argo::nargs(1)>()
                      .addArg<int, Argo::key("arg3"), true, Argo::nargs(1)>()
                      .addArg<int, Argo::key("arg4"), Argo::nargs(1), 'b', true>()
                      .addArg<int, Argo::key("arg5"), 'c', Argo::nargs(1), true>();

    EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
    // parser.parse(argc, argv);
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
                      .addPositionalArg<int, Argo::key("arg1"), Argo::nargs(3)>();

    parser.parse(argc, argv);
    EXPECT_THAT(parser.getArg<Argo::key("arg1")>(), testing::ElementsAre(1, 2, 3));
  }
  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "1", "2", "3",         //
        "--arg2", "42.195"               //
    );

    auto argo = Argo::Parser<131>("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<int, Argo::key("arg1"), Argo::nargs(3)>()
                      .addArg<float, Argo::key("arg2")>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<Argo::key("arg1")>(), testing::ElementsAre(1, 2, 3));
    EXPECT_FLOAT_EQ(parser.getArg<Argo::key("arg2")>(), 42.195);
  }

  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg2", "42.195",    //
        "1", "2", "3"                    //
    );

    auto argo = Argo::Parser<132>("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<int, Argo::key("arg1"), Argo::nargs(3)>()
                      .addArg<float, Argo::key("arg2"), Argo::nargs(1)>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<Argo::key("arg1")>(), testing::ElementsAre(1, 2, 3));
    EXPECT_FLOAT_EQ(parser.getArg<Argo::key("arg2")>(), 42.195);
  }

  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg2", "42", "96",  //
        "1", "2", "3"                    //
    );

    auto argo = Argo::Parser<133>("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<int, Argo::key("arg1"), Argo::nargs(3)>()
                      .addArg<float, Argo::key("arg2"), Argo::nargs(2)>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<Argo::key("arg1")>(), testing::ElementsAre(1, 2, 3));
    EXPECT_THAT(parser.getArg<Argo::key("arg2")>(), testing::ElementsAre(42, 96));
  }

  {
    auto [argc, argv] = createArgcArgv(  //
        "./main", "--arg2",              //
        "1", "2", "3"                    //
    );

    auto argo = Argo::Parser<134>("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<int, Argo::key("arg1"), Argo::nargs(3)>()
                      .addFlag<Argo::key("arg2")>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<Argo::key("arg1")>(), testing::ElementsAre(1, 2, 3));
    EXPECT_TRUE(parser.getArg<Argo::key("arg2")>());
  }

  {
    auto [argc, argv] = createArgcArgv(  //
        "./main",                        //
        "1", "2", "3", "-b"              //
    );

    auto argo = Argo::Parser<135>("Sample Program");
    auto parser = argo  //
                      .addPositionalArg<int, Argo::key("arg1"), Argo::nargs(3)>()
                      .addFlag<Argo::key("arg2"), 'b'>();

    parser.parse(argc, argv);

    EXPECT_THAT(parser.getArg<Argo::key("arg1")>(), testing::ElementsAre(1, 2, 3));
    EXPECT_TRUE(parser.getArg<Argo::key("arg2")>());
  }

  // { // error
  //   auto [argc, argv] = createArgcArgv(  //
  //       "./main",                        //
  //       "1", "2", "3", "-bc", "234.86"   //
  //   );
  //
  //   auto argo = Argo::Parser<136>("Sample Program");
  //   auto parser = argo  //
  //                     .addPositionalArg<int, Argo::key("arg1"), Argo::nargs(3)>()
  //                     .addFlag<Argo::key("arg2"), 'b'>()
  //                     .addArg<float, Argo::key("arg3"), 'c'>();
  //
  //   parser.parse(argc, argv);
  //
  //   EXPECT_THAT(parser.getArg<Argo::key("arg1")>(), testing::ElementsAre(1, 2, 3));
  //   EXPECT_TRUE(parser.getArg<Argo::key("arg2")>());
  //   EXPECT_FLOAT_EQ(parser.getArg<Argo::key("arg3")>(), 234.86);
  // }
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
//                       .addPositionalArg<int, Argo::key("arg1"), 'a', Argo::nargs(3)>();
//
//     parser.parse(argc, argv);
//     EXPECT_THAT(parser.getArg<Argo::key("arg1")>(), testing::ElementsAre(1, 2, 3));
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
    auto parser =
        argo.addPositionalArg<int, Argo::key("arg1"), Argo::nargs(3)>([](auto key, auto value) {
          EXPECT_EQ(key, "arg1");
          EXPECT_THAT(value, testing::ElementsAre(1, 2, 3));
        });

    parser.parse(argc, argv);
    EXPECT_THAT(parser.getArg<Argo::key("arg1")>(), testing::ElementsAre(1, 2, 3));
  }
}
