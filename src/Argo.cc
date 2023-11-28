module;

export module Argo;

export import :Exceptions;
export import :Parser;
export import :Validation;
export import :Initializer;

import :std_module;

export namespace Argo {

template <typename SubCommands = std::tuple<>>
struct Argo {};

};  // namespace Argo
