import std_modules;

auto main() -> int {
#ifndef NDEBUG
  std::cout << "Hello World DBUG" << std::endl;
#else
  std::cout << "Hello World" << std::endl;
#endif
  return 0;
}