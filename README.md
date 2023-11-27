# Argo
C++ mordern argument parser, using C++ module and metaprogramming.

## Feature
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
  auto argo = Argo::Parser();

  auto parser = argo
                    .addArg<int, Argo::key("arg1")>()
                    .addArg<float, Argo::key("arg2")>(53.4)
                    .addArg<std::string, Argo::key("arg3")>();

  parser.parse(argc, argv);

  std::println("{}", parser.getArg<Argo::key("arg1")>()); // 42
  std::println("{}", parser.getArg<Argo::key("arg2")>()); // 53.4 (default value)
  std::println("{}", parser.getArg<Argo::key("arg3")>()); // Hello,World
  // std::println("{}", parser.getArg<Argo::key("arg4")>()); // error

  // Static Typing
  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::key("arg1")>()), int>);
  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::key("arg2")>()), float>);
  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::key("arg3")>()), std::string>);

  return 0;
}
```

## How do I create multiple parser
Since `Argo` creates types for each argument and stores variables in those types,
the same ID in the parser might lead to conflicts with arguments.
Sometimes, this may result in overwriting the variables of another parser.
To avoid this, you can specify a unique ID.

```cpp
auto argo = Argo::Parser<42>();
```

## Usage(CMake)

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
- [ ] Description and help auto generation
- [ ] Standard validations
- [ ] Custom class casting
- [ ] sub command
