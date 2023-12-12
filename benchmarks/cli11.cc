#if CLI11_FOUND
#include <CLI/CLI.hpp>
#endif

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
#if CLI11_FOUND
  CLI::App app{"App description"};

  std::array<int, 8> arg1;
  app.add_option("--arg1", arg1);
  float arg2;
  app.add_option("--arg2", arg2);
  app.add_flag("--arg3");
  std::string arg4;
  app.add_option("--arg4", arg4);

  app.add_flag("-b,--arg5");
  app.add_flag("-c,--arg6");

  float arg7;
  app.add_option("-d,--arg7", arg7);

  app.add_flag("-e,--arg8");
  app.add_flag("-f,--arg9");
  app.add_flag("-g,--arg10");
  app.add_flag("-i,--arg12");
  app.add_flag("-j,--arg13");
  app.add_flag("-k,--arg14");
  app.add_flag("-l,--arg15");
  app.add_flag("-m,--arg16");
  app.add_flag("-n,--arg17");

  std::vector<int> arg18;
  app.add_option("--arg18", arg18);

  // CLI11_PARSE(app, argc, argv);
  std::cout << app.help() << '\n';
#endif
}
