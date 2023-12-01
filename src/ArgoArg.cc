module;

export module Argo:Arg;

import :NArgs;
import :Validation;
import :TypeTraits;
import :std_module;

export namespace Argo {

template <std::size_t N>
struct ParserID {
  int idInt = 0;
  char idName[N];

  constexpr ParserID(int id) : idInt(id){};

  constexpr ParserID(const char (&id)[N + 1]) {
    for (std::size_t i = 0; i < N; i++) {
      this->idName[i] = id[i];
    }
  };
};

ParserID(int) -> ParserID<0>;

template <std::size_t N>
ParserID(const char (&)[N]) -> ParserID<N - 1>;

struct ArgNameTag {};

template <std::size_t N>
struct ArgName : ArgNameTag {
  char name[N] = {};
  char shortName = '\0';
  std::size_t nameLen = N;

  explicit ArgName() = default;

  constexpr ArgName(const char (&lhs)[N + 1]) {
    for (std::size_t i = 0; i < N; i++) {
      if (lhs[i] == ',') {
        nameLen = i;
        shortName = lhs[i + 1];
        return;
      }
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
    return &this->name[this->nameLen];
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
    auto NV = this->nameLen;
    auto MV = lhs.nameLen;

    if (MV != NV) {
      return false;
    } else {
      for (std::size_t i = 0; i < NV; i++) {
        if ((*this)[i] != lhs[i]) {
          return false;
        }
      }
      return true;
    }
  }

  constexpr operator std::string_view() const {
    return std::string_view(this->begin(), this->end());
  }

  constexpr auto containsInvalidChar() const -> bool {
    auto invalid_chars = std::string_view(" \\\"'<>&|$[]");
    if (invalid_chars.contains(this->shortName)) {
      return true;
    }
    for (std::size_t i = 0; i < this->nameLen; i++) {
      if (invalid_chars.contains(this->name[i])) {
        return true;
      }
    }
    return false;
  }

  constexpr auto hasValidNameLength() const -> bool {
    if (this->shortName == '\0') {
      return true;
    }
    return (N - this->nameLen) == 2;
  }
};

template <std::size_t N>
consteval std::size_t calcN(const char (&)[N]) {
  return 4;
}

template <std::size_t N>
ArgName(const char (&)[N]) -> ArgName<N - 1>;

template <class T>
concept ArgType = requires(T& x) {
  std::derived_from<decltype(T::name), ArgNameTag>;
  std::is_same_v<decltype(T::isVariadic), char>;
  std::is_same_v<decltype(T::nargs), NArgs>;
  typename T::baseType;
  typename T::type;
  not std::is_same_v<decltype(T::value), void>;
  std::is_same_v<decltype(T::assigned), bool>;
  std::is_same_v<decltype(T::description), std::string_view>;
};

template <typename BaseType, ArgName Name, ParserID ID>
struct ArgBase {
  static constexpr auto name = Name;
  static constexpr auto id = ID;
  inline static bool assigned = false;
  inline static std::string_view description;
  using baseType = BaseType;
};

struct ArgTag {};

/*!
 * Arg type this holds argument value
 */
template <class Type, ArgName Name, NArgs TNArgs, bool Required, ParserID ID>
struct Arg : ArgTag, ArgBase<Type, Name, ID> {
  static constexpr bool isVariadic =
      (TNArgs.nargs > 1) || (TNArgs.nargs_char == '+') || (TNArgs.nargs_char == '*');
  static constexpr bool isFixedLength = (TNArgs.nargs > 1);
  using type =                                                           //
      std::conditional_t<                                                //
          !isVariadic                                                    //
              || is_array_v<Type>                                        //
              || is_tuple_v<Type>                                        //
              || is_vector_v<Type>,                                      //
          Type,                                                          //
          std::conditional_t<                                            //
              isFixedLength,                                             //
              std::array<Type, static_cast<std::size_t>(TNArgs.nargs)>,  //
              std::vector<Type>                                          //
              >                                                          //
          >;                                                             //

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = TNArgs;
  inline static Validation::ValidationBase<type>* validator = nullptr;
  inline static std::function<Type(std::string_view)> caster = nullptr;
  inline static std::function<void(type, std::string_view)> callback = nullptr;

  inline static bool required = Required;
};

struct FlagArgTag {};

template <ArgName Name, ParserID ID>
struct FlagArg : FlagArgTag, ArgBase<bool, Name, ID> {
  static constexpr bool isVariadic = false;
  using type = bool;

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = NArgs(-1);
  inline static std::function<void()> callback = nullptr;
};

}  // namespace Argo
