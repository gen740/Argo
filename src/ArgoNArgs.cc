module;

import std_module;
export module Argo:NArgs;

export namespace Argo {

/*!
 * (default)?  : If value specified use it else use default -> ValueType
 *          int: Exactly (n > 1)                            -> vector<ValueType>
 *          *  : Any number of argument if zero use default -> vector<ValueType>
 *          +  : Any number of argument except zero         -> vector<ValueType>
 */
struct NArgs {
  int nargs = -1;
  char nargs_char = '?';

  template <int T>
    requires(T >= 1)
  constexpr explicit NArgs() : nargs(T) {}

  template <char T>
    requires(T == '?' or T == '+' or T == '*')
  constexpr explicit NArgs() : nargs(T) {}

  constexpr explicit NArgs(int narg) : nargs(narg) {
    if (narg <= 0) {
      throw std::invalid_argument(std::format("Nargs should be larger than 0, got {}", narg));
    }
  }

  constexpr explicit NArgs(char narg) : nargs_char(narg) {
    if (narg != '?' && narg != '*' && narg != '+') {
      throw std::invalid_argument(std::format("Nargs should be '?', '*' or '+', got {}", narg));
    }
  }

 private:
  template <class Type, auto Name, char ShortName, int ID>
  friend struct Arg;
};
};  // namespace Argo
