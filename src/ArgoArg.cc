module;

import std_module;
export module Argo:Arg;
import :NArgs;
import :Validation;

export namespace Argo {

/*!
 * Arg type this holds argument value
 */
template <class Type, auto Name, char ShortName, NArgs TNArgs, int ID>
struct Arg {
  static constexpr auto name = Name;
  static constexpr char shortName = ShortName;
  using type = Type;
  inline static std::optional<type> value = {};
  inline static std::optional<type> defaultValue = {};
  inline static std::string_view description;
  inline static constexpr NArgs nargs = TNArgs;
  inline static Validation::ValidationBase<Type>* validator = nullptr;
  inline static std::function<Type(std::string_view)> caster = nullptr;
};

template <auto Name, char ShortName, int ID>
struct FlagArg {
  static constexpr auto name = Name;
  static constexpr char shortName = ShortName;
  using type = bool;
  inline static type value = false;
  inline static std::string_view description;
};

}  // namespace Argo
