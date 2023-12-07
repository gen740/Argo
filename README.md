[![Tests](https://github.com/gen740/Argo/actions/workflows/test.yml/badge.svg)](https://github.com/gen740/Argo/actions/workflows/test.yml)

# Argo
C++ mordern and safest argument parser, using C++ modules and metaprogramming.

## Features

1. **Static Type Argument Parsing**: Ensures command line arguments are parsed with strict type adherence, enhancing reliability.

2. **Type Safety for All Flags**: Provides robust type safety for command line flags, reducing runtime errors and ensuring type preservation during retrieval.

3. **Compile-Time Duplicate Argument Detection**: Detects duplicate flags at compile time, serving a better development experience.

4. **Rich Feature Set**: Offers a comprehensive array of features for versatile command line parsing needs.

5. **Fastest Parsing Mechanism**: Delivers high-speed parsing, ideal for performance-critical applications.

6. **Compile-Time Calculations**: Performs certain operations at compile time for enhanced performance efficiency. 


## Table of Contents
1. [**Requirements**](#requirements)
2. [**Basic Example**](#basic-example)
3. [**Defining Arguments**](#defining-arguments)
4. [**Template Options**](#template-options)
   - [ShortKey](#shortkey)
   - [Required](#required)
   - [nargs](#nargs)
5. [**Other Options**](#other-options)
   - [Implicit/Explicit Default](#implicitexplicit-default)
   - [Description](#description)
   - [Callback](#callback)
   - [STL Support](#stl-support)
6. [**Creating Multiple Parsers**](#creating-multiple-parsers)
7. [**Adding Subcommands**](#adding-subcommands)
   - [Parsing Results](#parsing-results)
8. [**Installation with CMake**](#installation-with-cmake)
   - [Using Submodule](#using-submodule)
   - [Using FetchContent](#using-fetchcontent)

## Requirement
- **C++23 compiler** clang >= 17
- **libcxx standard library**
- **C++ Modules supported CMake** cmake >= 3.28


## Basic Example
```cpp
import Argo;
import std;

// suppose ./main --arg1 42 --arg3 "Hello,World"
auto main(int argc, char* argv[]) -> int {

    auto parser = Argo::Parser()  //
                    .addArg<"arg1", int>()
                    .addArg<"arg2", float>(Argo::explicitDefault(12.34))
                    .addArg<"arg3", std::string>();

  parser.parse(argc, argv);

  std::println("{}", parser.getArg<"arg1">()); // 42
  std::println("{}", parser.getArg<"arg2">()); // 12.34 (default value)
  std::println("{}", parser.getArg<"arg3">()); // Hello,World

  // std::println("{}", parser.getArg<"arg4">()); // compile error

  // Static Typing
  static_assert(
      std::is_same_v<decltype(parser.getArg<"arg1">()), int>);
  static_assert(
      std::is_same_v<decltype(parser.getArg<"arg2">()), float>);
  static_assert(
      std::is_same_v<decltype(parser.getArg<"arg3">()), std::string>);

  return 0;
}
```
[Try this at Compiler Explorer](https://godbolt.org/z/rfMsPnroc).

## Defining argument

Defining a parser in Argo can be somewhat intricate. It is not accurate to define it as shown below:

```cpp
auto parser = Argo::Parser();
parser.addArg<"arg1", int>();
parser.addArg<"arg2", int>();

parser.parse(argc, argv);
```

In Argo, arguments are stored in the return types of the `addArg` method. Therefore, the correct declaration should be as follows:

```cpp
auto argo = Argo::Parser();
auto pre_parser = argo.addArg<"arg1", int>();
auto parser = argo.addArg<"arg2", int>();

parser.parse(argc, argv);
```

This approach ensures that each argument is properly added and stored, enabling the parser to function as expected.

## Template Options

### ShortKey

Specifying a `char` character as the `key` parameter designates the abbreviated
version of the argument flag. In the provided code, the flag `-a` can be passed
to the parser.

Example:
```cpp
Argo::Parser().addArg<"arg1,a", Type>();
```

### Required
Specifying `true` as the addArg template parameter indicates that the argument
will be `required`. The parser will throw an exception if that argument is not
specified.

Example:
```cpp
Argo::Parser().addArg<"arg1", Type, true>();
```

### nargs

By using `Argo::nargs` as a template parameter in `Argo::Parser`, you can
define the `nargs` for a particular flag. Below is a table detailing the
options for `nargs`.

| narg | number of values | description |
|------|------------------|-------------|
| int  | exactly n        | Stores exactly n values in the `std::vector<Type>`. |
| '?'  | zero or one      | An implicit default value is used if zero values are provided; an explicit value is set when the flag is specified. |
| '+'  | more than one    | Stores n values in the `std::vector<Type>`. |
| '*'  | zero or more     | An implicit default value is used if zero values are provided; otherwise, behaves the same as '+'. |

Example usage:
```cpp
Argo::Parser().addArg<"arg1", Type, nargs('+')>();
```
This template parameter configuration specifies that the `arg1` flag accepts
one or more arguments, which will be stored in a `std::vector<Type>`.

---

`required` and `nargs` values are interchangable you can specify these value in the same time.

```cpp
Argo::Parser().addArg<"arg1", Type, true, nargs('+')>();
Argo::Parser().addArg<"arg1", Type, nargs('+'), true>();
```

These are all the same.


## Other Options

### Implicit/Explicit Default
You can specify the implicit or explicit default value as follows:

- Implicit Default
  ```cpp
  Argo::Parser().addArg<"arg1", int>(Argo::implicitDefault(12.34));
  ```
- Explicit Default
  ```cpp
  Argo::Parser().addArg<"arg1", int>(Argo::explicitDefault(12.34));
  ```

### Description
Specify a description for the argument like this:
  ```cpp
  Argo::Parser().addArg<"arg1", int>(Argo::withDescription("Description of arg1"));
  ```

### Callback
Set a callback for the flag, which is triggered as soon as the option is parsed:
  ```cpp
  Argo::Parser().addArg<"arg1", int>([](auto value, auto raw_value){
      // Implement some callback using value and raw_value
      // value would be reference to the value, so you can modify them.
  });
  ```
In this callback, the parsed value is modifiable. You can validate the value inside this function.
  ```cpp
  namespace fs = std::filesystem;
  Argo::Parser(),addArg<"file_path", fs::path>([](fs::path& value, std::span<std::string_view> /* unused */){
      value = fs::absolute(value);
      if (!fs::exists(value)) {
          throw Argo::ValidationError(std::format("file {} does not exist"), value);
      }
  });
  ```

### STL Support

You can use `std::vector` and `std::array` or `std::tuple` for type,
specifying these types, Argo guess the nargs and automatically set it for you.

```cpp
// suppose ./main.a --arg1 42 3.14 "Hello,World"
auto argo = Argo::Parser();
auto parser = argo.addArg<"arg1", std::tuple<int, double, std::string>>();
parser.parse(argc, argv);

auto [a1, a2, a3] = parser.getArg<"arg1">(); // 42 3.14 "Hello,World"
```

## How to Create Multiple Parsers

Because `Argo` generates types for each argument and stores variables within
these types, using the same ID in multiple parsers might cause conflicts with
the arguments. This conflict can sometimes lead to variables in one parser
being overwritten by another. To prevent this, you should assign a unique ID.

```cpp
auto argo1 = Argo::Parser<42>();
auto argo2 = Argo::Parser<"unique ID">();
```

## Adding Subcommands

```cpp
#include <Argo.h>

int main() {
    auto parser1 = Argo::Parser<"SubCommands1_cmd1">()
                       .addArg<"arg1,a", int>()
                       .addArg<"arg2,b", int>(Argo::explicitDefault(123));
    auto parser2 = Argo::Parser<"SubCommands1_cmd2">()
                       .addArg<"arg3,a", int>()
                       .addArg<"arg4,b", int>();

    auto parser = Argo::Parser<"SubCommands1">()
                      .addParser<"cmd1">(parser1, Argo::description("cmd1"))
                      .addParser<"cmd2">(parser2, Argo::description("cmd2"));

    parser.parse(argc, argv);
}
```

To integrate a subparser into your main parser, use the `addParser` method. The
template argument for this method will be the name of the subcommand. With this
setup, you can easily handle command line arguments like `cmd1 --arg1 42` or
`cmd2 -b 24`.

### Parsing Results

To obtain parse results:

To access a subcommand parser, use `getParser<Identifier>()`. This returns the
same parser object that was initially added with `addParser`. To determine if a
particular subcommand was invoked, evaluate the truth of the respective parser
object.

```cpp
if (parser1) {
    println("{}", parser1.getArg<"arg1">());
}
```

## Installation(CMake)

### Using submodule
1. clone this repository into you project
```
git clone https://github.com/gen740/Argo.git
```
2. Add subdirectory
```camke
add_subdirectory("path/to/Argo")
target_link_libraries(YOUR_TARGET Argo)
```

### Using FetchContent
```cmake
include(FetchContent)

FetchContent_Declare(
  Argo
  GIT_REPOSITORY https://github.com/gen740/Argo.git
  GIT_TAG        main  # or use a specific commit/tag
)

FetchContent_MakeAvailable(Argo)

target_link_libraries(YOUR_TARGET PRIVATE Argo)
```

### Single Header (experimental)
You can use this library as a single header file.
Just copy and paste `include/Argo.hh` into your library.

## Roadmap
- [x] Boolean Flag support
- [x] Support for short option
- [x] Combining Boolean Flag
- [x] Validation
- [x] Support for multiple parser
- [x] Argument duplication validation
- [x] Optional argument
- [x] "narg" argument
- [x] Required argument
- [x] Positional argument
- [x] sub command
- [ ] Description and help auto generation
- [ ] Standard validations
- [ ] Custom class casting
