#!/usr/bin/env sh

set -e

CLANG_TIDY_FLAGS="--quiet -p ./build"
CLANG_TIDY=clang-tidy

# if clang-format not found, exit
if [ -z "$CLANG_TIDY" ]; then
  # bold red error message
  printf '\033[1;31mError:\033[0m clang-tidy not found\n' >&2
  exit 1
fi


# set find command according to OS
if [ "$(uname)" = "Darwin" ]; then
  alias find_command='/usr/bin/find -E . -iregex ".*\.(cpp|hpp|cc|cxx|ipp|h)"'
else
  alias find_command='/usr/bin/find . -regex ".*\.\(cpp\|hpp\|cc\|cxx\|ipp\|h\)"'
fi

find_command \
  -not -path './.git/*' \
  -not -path './.gitlab/*' \
  -not -path './.cache/*' \
  -not -path './.venv/*' \
  -not -path './venv/*' \
  -not -path './node_modules/*' \
  -not -path './external/*' \
  -not -path './sim/*' \
  -not -path './multiple_test/*' \
  -not -path './build/*' \
  -not -path './tests/*' \
  -not -path './benchmarks/*' |
  xargs -n 1 -P 8 -I {} $CLANG_TIDY $CLANG_TIDY_FLAGS {}

unset CLANG_TIDY
unset CLANG_TIDY_FLAGS
unalias find_command
