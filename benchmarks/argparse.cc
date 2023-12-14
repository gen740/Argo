#if argparse_FOUND
#include <argparse/argparse.hpp>
#endif

auto main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> int {
#if argparse_FOUND
  argparse::ArgumentParser program("program_name");
  program.add_argument("--arg1").nargs(8).scan<'d', int>();
  program.add_argument("--arg2").scan<'g', double>();
  program.add_argument("--arg3").default_value(false);
  program.add_argument("--arg4").nargs(1);
  program.add_argument("-b", "--arg5").default_value(false);
  program.add_argument("-c", "--arg6").default_value(false);
  program.add_argument("-d", "--arg7");
  program.add_argument("-e", "--arg8").default_value(false);
  program.add_argument("-f", "--arg9").default_value(false);
  program.add_argument("-g", "--arg10").default_value(false);
  program.add_argument("-i", "--arg11").default_value(false);
  program.add_argument("-j", "--arg12").default_value(false);
  program.add_argument("-k", "--arg13").default_value(false);
  program.add_argument("-l", "--arg14").default_value(false);
  program.add_argument("-m", "--arg15").default_value(false);
  program.add_argument("-n", "--arg16").default_value(false);
  program.add_argument("--arg18").nargs(100).scan<'d', int>();

  program.parse_args(argc, argv);
#endif
  return 0;
}
