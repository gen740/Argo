# Argo
C++ mordern argument parser, using C++ module and metaprogramming.

## Features
- Static type argument parse
- Detect duplicate argument in compile time
- Rich feature

## Requirement
- **C++23 compiler** clang >= 17
- **C++ Modules supported CMake** cmake >= 3.28

## Example
```cpp
import Argo;
import std;

// suppose ./main --arg1 42 --arg3 "Hello,World"
auto main(int argc, char* argv[]) -> int {

    auto parser = Argo::Parser("Program name")  //
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
Argo::Parser("Program").addArg<"arg1,a", Type>();
```

### Required
Specifying `true` as the addArg template parameter indicates that the argument
will be `required`. The parser will throw an exception if that argument is not
specified.

Example:
```cpp
Argo::Parser("Program").addArg<"arg1", Type, true>();
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
Argo::Parser("Program").addArg<"arg1", Type, nargs('+')>();
```
This template parameter configuration specifies that the `arg1` flag accepts
one or more arguments, which will be stored in a `std::vector<Type>`.

---

`required` ond `nargs` values are interchangable you can specify these value in the same time.

```cpp
Argo::Parser("Program").addArg<"arg1", Type, true, nargs('+')>();
Argo::Parser("Program").addArg<"arg1", Type, nargs('+'), true>();
```

These are all the same.


## Other Options

### Implicit/Explicit Default
You can specify the implicit or explicit default value as follows:

- Implicit Default
  ```cpp
  Argo::Parser("Program").addArg<"arg1", int>(Argo::implicitDefault(12.34));
  ```
- Explicit Default
  ```cpp
  Argo::Parser("Program").addArg<"arg1", int>(Argo::explicitDefault(12.34));
  ```

### Description
Specify a description for the argument like this:
  ```cpp
  Argo::Parser("Program").addArg<"arg1", int>(Argo::withDescription("Description of arg1"));
  ```

### Validator
Define a validator for the argument, for example, a range validator:
  ```cpp
  Argo::Parser("Program").addArg<"arg1", int>(Argo::validator::MinMax(10, 20));
  ```

### Callback
Set a callback for the flag, which is triggered as soon as the option is parsed:
  ```cpp
  Argo::Parser("Program").addArg<"arg1", int>([](auto value, auto raw_value){
      // Implement some callback using value and raw_value
  });
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
