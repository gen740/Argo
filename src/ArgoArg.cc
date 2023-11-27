module;

export module Argo:Arg;

import :NArgs;
import :Validation;
import :std_module;

export namespace Argo {

template <class T>
concept ArgType = requires(T& x) {
  std::is_array_v<decltype(T::name)>;
  std::is_same_v<decltype(T::shortName), char>;
  std::is_same_v<decltype(T::isVariadic), char>;
  std::is_same_v<decltype(T::nargs), NArgs>;
  typename T::baseType;
  typename T::type;
  not std::is_same_v<decltype(T::value), void>;
  std::is_same_v<decltype(T::assigned), bool>;
  std::is_same_v<decltype(T::description), std::string_view>;
};

template <typename BaseType, auto Name, char ShortName, int ID>
struct ArgBase {
  static constexpr auto name = Name;
  static constexpr char shortName = ShortName;
  static constexpr int id = ID;
  inline static bool assigned = false;
  inline static std::string_view description;
  using baseType = BaseType;
};

struct ArgTag {};

/*!
 * Arg type this holds argument value
 */
template <class Type, auto Name, char ShortName, NArgs TNArgs, bool Required, int ID>
struct Arg : ArgTag, ArgBase<Type, Name, ShortName, ID> {
  static constexpr bool isVariadic =
      (TNArgs.nargs > 1) || (TNArgs.nargs_char == '+') || (TNArgs.nargs_char == '*');
  using type = std::conditional_t<  //
      isVariadic,                   //
      std::vector<Type>,            //
      Type>;

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = TNArgs;
  inline static Validation::ValidationBase<type>* validator = nullptr;
  inline static std::function<Type(std::string_view)> caster = nullptr;

  inline static constexpr bool required = Required;
};

struct FlagArgTag {};

template <auto Name, char ShortName, int ID>
struct FlagArg : FlagArgTag, ArgBase<bool, Name, ShortName, ID> {
  static constexpr bool isVariadic = false;
  using type = bool;

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = NArgs(-1);
};

}  // namespace Argo
