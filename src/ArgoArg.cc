module;

import std_module;
export module Argo:Arg;
import :NArgs;
import :Validation;

export namespace Argo {

struct ArgTag {};

/*!
 * Arg type this holds argument value
 */
template <class Type, auto Name, char ShortName, NArgs TNArgs, int ID>
struct Arg : ArgTag {
  static constexpr auto name = Name;
  static constexpr char shortName = ShortName;
  static constexpr bool isVariadic =
      (TNArgs.nargs > 1) || (TNArgs.nargs_char == '+') || (TNArgs.nargs_char == '*');
  using baseType = Type;
  using type = std::conditional_t<  //
      isVariadic,                   //
      std::vector<Type>,            //
      Type>;
  inline static std::optional<type> value = {};
  inline static std::optional<type> defaultValue = {};
  inline static std::string_view description;
  inline static constexpr NArgs nargs = TNArgs;
  inline static Validation::ValidationBase<type>* validator = nullptr;
  inline static std::function<Type(std::string_view)> caster = nullptr;
};

struct FlagArgTag {};

template <auto Name, char ShortName, int ID>
struct FlagArg : FlagArgTag {
  static constexpr auto name = Name;
  static constexpr char shortName = ShortName;
  static constexpr bool isVariadic = false;
  using baseType = bool;
  using type = bool;
  inline static type value = false;
  inline static std::string_view description;
};

}  // namespace Argo
