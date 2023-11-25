module;

import std_module;

export module Argo:NArgs;

export namespace Argo {

constexpr char NULLCHAR = '\0';

/*!
 * (default)?  : If value specified use it else use default -> ValueType
 *          int: Exactly (n > 1)                            -> vector<ValueType>
 *          *  : Any number of argument if zero use default -> vector<ValueType>
 *          +  : Any number of argument except zero         -> vector<ValueType>
 */
struct NArgs {
  int nargs = -1;
  char nargs_char = NULLCHAR;

  constexpr explicit NArgs(char arg) : nargs_char(arg) {}

  constexpr explicit NArgs(int arg) : nargs(arg) {}
};

};  // namespace Argo
