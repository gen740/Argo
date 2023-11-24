import Argo;

#include <gtest/gtest.h>

#include <string>
#include <type_traits>

using Argo::InvalidArgument;

TEST(ArgoTest, ExampleCode) {
  const int argc = 5;
  char* argv[argc] = {"./main", "--arg1", "42", "--arg3", "Hello,World"};

  auto argo = Argo::Parser();
  auto parser = argo.addArg<int, Argo::arg("arg1")>()
                    .addArg<float, Argo::arg("arg2")>(53.4)
                    .addArg<std::string, Argo::arg("arg3")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::arg("arg1")>(), 42);
  EXPECT_FLOAT_EQ(parser.getArg<Argo::arg("arg2")>(), 53.4);
  EXPECT_EQ(parser.getArg<Argo::arg("arg3")>(), "Hello,World");

  EXPECT_TRUE((std::is_same_v<decltype(parser.getArg<Argo::arg("arg1")>()), int>));
  EXPECT_TRUE((std::is_same_v<decltype(parser.getArg<Argo::arg("arg2")>()), float>));
  EXPECT_TRUE((std::is_same_v<decltype(parser.getArg<Argo::arg("arg3")>()), std::string>));
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

  auto argo = Argo::Parser();
  auto parser = argo.addArg<int, Argo::arg("arg1")>()
                    .addArg<float, Argo::arg("arg2")>()
                    .addArg<double, Argo::arg("arg3")>()
                    .addArg<std::string, Argo::arg("arg4")>()
                    .addArg<const char*, Argo::arg("arg5")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::arg("arg1")>(), 42);
  EXPECT_FLOAT_EQ(parser.getArg<Argo::arg("arg2")>(), 42.1234567890f);
  EXPECT_DOUBLE_EQ(parser.getArg<Argo::arg("arg3")>(), 42.12345678901234567890);
  EXPECT_EQ(parser.getArg<Argo::arg("arg4")>(), "Hello,World!");
  EXPECT_EQ(parser.getArg<Argo::arg("arg5")>(), "Hello,World!const char*");

  EXPECT_TRUE(  //
      (std::is_same_v<decltype(parser.getArg<Argo::arg("arg1")>()), int>));
  EXPECT_TRUE(  //
      (std::is_same_v<decltype(parser.getArg<Argo::arg("arg2")>()), float>));
  EXPECT_TRUE(  //
      (std::is_same_v<decltype(parser.getArg<Argo::arg("arg3")>()), double>));
  EXPECT_TRUE(  //
      (std::is_same_v<decltype(parser.getArg<Argo::arg("arg4")>()), std::string>));
  EXPECT_TRUE(  //
      (std::is_same_v<decltype(parser.getArg<Argo::arg("arg5")>()), const char*>));
}

TEST(ArgoTest, ExceptionThow) {
  const int argc = 5;
  char* argv[argc] = {"./main", "arg1=42", "--arg2=23.4"};

  auto argo = Argo::Parser();
  auto parser = argo.addArg<int, Argo::arg("arg1")>()  //
                    .addArg<float, Argo::arg("arg2")>();

  EXPECT_THROW(parser.getArg<Argo::arg("arg1")>(), Argo::ParseError);
  EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
}

TEST(ArgoTest, EqualAssign) {
  const int argc = 3;
  char* argv[argc] = {"./main", "--arg1=42", "--arg2=Hello,World"};

  auto argo = Argo::Parser();
  auto parser = argo.addArg<int, Argo::arg("arg1")>().addArg<std::string, Argo::arg("arg2")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::arg("arg1")>(), 42);
  EXPECT_EQ(parser.getArg<Argo::arg("arg2")>(), "Hello,World");
}

TEST(ArgoTest, FlagArgument) {
  const int argc = 8;
  char* argv[argc] = {"./main", "--arg1", "--arg2=true", "--arg3=1",
                      "--arg4", "true",   "--arg5",      "1"};

  auto argo = Argo::Parser();
  auto parser = argo.addArg<bool, Argo::arg("arg1")>()
                    .addArg<bool, Argo::arg("arg2")>()
                    .addArg<bool, Argo::arg("arg3")>()
                    .addArg<bool, Argo::arg("arg4")>()
                    .addArg<bool, Argo::arg("arg5")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::arg("arg1")>(), true);
  EXPECT_EQ(parser.getArg<Argo::arg("arg2")>(), true);
  EXPECT_EQ(parser.getArg<Argo::arg("arg3")>(), true);
  EXPECT_EQ(parser.getArg<Argo::arg("arg4")>(), true);
  EXPECT_EQ(parser.getArg<Argo::arg("arg5")>(), true);
}
