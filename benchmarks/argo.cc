import Argo;

// #include <iostream>
#include <string>

using namespace Argo;

auto main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> int {
  __asm__("# Creating parser");
  auto parser = Parser<1>()  //
                    .addArg<"arg1", int, nargs(8)>()
                    .addArg<"arg2", float>()
                    .addFlag<"arg3">()
                    .addArg<"arg4", std::string, nargs(1)>()
                    .addFlag<"arg5,b">()
                    .addFlag<"arg6,c">()
                    .addArg<"arg7,d", float>()
                    .addFlag<"arg8,e">()
                    .addFlag<"arg9,f">()
                    .addFlag<"arg10,g">()
                    .addFlag<"arg12,i">()
                    .addFlag<"arg13,j">()
                    .addFlag<"arg14,k">()
                    .addFlag<"arg15,l">()
                    .addFlag<"arg16,m">()
                    .addFlag<"arg17,n">()
                    .addArg<"arg18", int, nargs('+')>();
  __asm__("# Creating parser end");
  // parser.parse(argc, argv);
  // std::cout << parser.formatHelp() << '\n';
  return 0;
}
