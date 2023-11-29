module;

export module Argo:Arg;

import :NArgs;
import :Validation;
import :std_module;

export namespace Argo {

struct ArgNameTag {};

template <std::size_t N>
struct ArgName : ArgNameTag {
  char name[N];
  char shortName = '\0';

  explicit ArgName() = default;

  constexpr ArgName(const char (&lhs)[N + 1]) {
    for (std::size_t i = 0; i < N; i++) {
      this->name[i] = lhs[i];
    }
  };

  constexpr ArgName(const char (&lhs)[N + 1], char short_name) : shortName(short_name) {
    for (std::size_t i = 0; i < N; i++) {
      this->name[i] = lhs[i];
    }
  };

  constexpr char operator[](std::size_t idx) const {
    return this->name[idx];
  }

  constexpr char& operator[](std::size_t idx) {
    return this->name[idx];
  }

  constexpr auto begin() const {
    return &this->name[0];
  }

  constexpr auto end() const {
    return &this->name[N - 1];
  }

  constexpr auto size() const {
    return N;
  }

  friend auto begin(ArgName lhs) {
    return lhs.begin();
  }

  friend auto end(ArgName lhs) {
    return lhs.end();
  }

  template <std::size_t M>
  constexpr auto operator==(ArgName<M> lhs) -> bool {
    if constexpr (M != N) {
      return false;
    } else {
      for (std::size_t i = 0; i < N; i++) {
        if ((*this)[i] != lhs[i]) {
          return false;
        }
      }
      return true;
    }
  }

  template <std::size_t M>
  constexpr auto operator==(ArgName<M> lhs) const -> bool {
    if constexpr (M != N) {
      return false;
    } else {
      for (std::size_t i = 0; i < N; i++) {
        if ((*this)[i] != lhs[i]) {
          return false;
        }
      }
      return true;
    }
  }
};

template <class T>
concept ArgType = requires(T& x) {
  std::is_same_v<decltype(T::shortName), char>;
  std::is_same_v<decltype(T::isVariadic), char>;
  std::is_same_v<decltype(T::nargs), NArgs>;
  typename T::baseType;
  typename T::type;
  not std::is_same_v<decltype(T::value), void>;
  std::is_same_v<decltype(T::assigned), bool>;
  std::is_same_v<decltype(T::description), std::string_view>;
};

template <typename BaseType, auto Name, char ShortName, auto ID>
struct ArgBase {
  static constexpr auto name = Name;
  static constexpr char shortName = ShortName;
  static constexpr auto id = ID;
  inline static bool assigned = false;
  inline static std::string_view description;
  using baseType = BaseType;
};

struct ArgTag {};

/*!
 * Arg type this holds argument value
 */
template <class Type, auto Name, char ShortName, NArgs TNArgs, bool Required, auto ID>
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
  inline static std::function<void(type, std::string_view)> callback = nullptr;

  inline static constexpr bool required = Required;
};

struct FlagArgTag {};

template <auto Name, char ShortName, auto ID>
struct FlagArg : FlagArgTag, ArgBase<bool, Name, ShortName, ID> {
  static constexpr bool isVariadic = false;
  using type = bool;

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = NArgs(-1);
  inline static std::function<void(type, std::string_view)> callback = nullptr;
};

}  // namespace Argo
