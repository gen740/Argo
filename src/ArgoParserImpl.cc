module;

export module Argo:ParserImpl;

import :Parser;
import :MetaLookup;

export namespace Argo {

template <auto ID, class Args, class PositionalArg, bool HelpEnabled>
auto Parser<ID, Args, PositionalArg, HelpEnabled>::addHelp() -> void {
  static_assert((SearchIndexFromShortName<Arguments, 'h'>::value == -1), "Duplicated short name");
  static_assert(Argo::SearchIndex<Arguments, key("help")>::value == -1, "Duplicated name");
  return Parser<ID, Arguments, PositionalArgument, true>();
}

}  // namespace Argo
