# Argo
C++ mordern argument parser, using C++ module and metaprogramming.

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
                    .addArg<int, Argo::arg("arg1")>()
                    .addArg<float, Argo::arg("arg2")>(53.4)
                    .addArg<std::string, Argo::arg("arg3")>();

  parser.parse(argc, argv);

  std::println("{}", parser.getArg<Argo::arg("arg1")>()); // 42
  std::println("{}", parser.getArg<Argo::arg("arg2")>()); // 53.4 (default value)
  std::println("{}", parser.getArg<Argo::arg("arg3")>()); // Hello,World
  // std::println("{}", parser.getArg<Argo::arg("arg4")>()); // error

  // Static Typing
  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg1")>()), int>);
  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg2")>()), float>);
  static_assert(
      std::is_same_v<decltype(parser.getArg<Argo::arg("arg3")>()), std::string>);

  return 0;
}
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
  GIT_TAG        master  # or use a specific commit/tag
)

FetchContent_MakeAvailable(Argo)

target_link_libraries(YOUR_TARGET PRIVATE Argo)
```

## Roadmap
- [x] Boolean Flag support
- [x] Support for short option
- [ ] Combining Boolean Flag
- [ ] Validation
- [ ] Custom class casting
- [ ] Argument duplication validation
- [ ] Optional argument
- [ ] Description and help auto generation
- [ ] "narg" argument
- [ ] Positional argument
