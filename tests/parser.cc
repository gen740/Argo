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

  EXPECT_TRUE(
      (std::is_same_v<decltype(parser.getArg<Argo::arg("arg1")>()), int>));
  EXPECT_TRUE(
      (std::is_same_v<decltype(parser.getArg<Argo::arg("arg2")>()), float>));
  EXPECT_TRUE((std::is_same_v<decltype(parser.getArg<Argo::arg("arg3")>()),
                              std::string>));
}

TEST(ArgoTest, ExceptionThow) {
  const int argc = 5;
  char* argv[argc] = {"./main", "arg1=42", "--arg2=23,4"};

  auto argo = Argo::Parser();
  auto parser = argo.addArg<int, Argo::arg("arg1")>()  //
                    .addArg<float, Argo::arg("arg2")>();

  EXPECT_THROW(parser.parse(argc, argv), Argo::InvalidArgument);
}

TEST(ArgoTest, EqualAssign) {
  const int argc = 3;
  char* argv[argc] = {"./main", "--arg1=42", "--arg2=Hello,World"};

  auto argo = Argo::Parser();
  auto parser = argo.addArg<int, Argo::arg("arg1")>()
                    .addArg<std::string, Argo::arg("arg2")>();

  parser.parse(argc, argv);

  EXPECT_EQ(parser.getArg<Argo::arg("arg1")>(), 42);
  EXPECT_EQ(parser.getArg<Argo::arg("arg2")>(), "Hello,World");
}
