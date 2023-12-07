module;

export module Argo:Arg;

import :NArgs;
import :Validation;
import :TypeTraits;
import :std_module;

// generator start here

export namespace Argo {

template <std::size_t N>
struct ParserID {
  int idInt = 0;
  char idName[N];

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr ParserID(int id) : idInt(id){};

  // NOLINTNEXTLINE(google-explicit-constructor)
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

  // NOLINTNEXTLINE(google-explicit-constructor)
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
    }
    for (std::size_t i = 0; i < NV; i++) {
      if ((*this)[i] != lhs[i]) {
        return false;
      }
    }
    return true;
  }

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr operator std::string_view() const {
    return std::string_view(this->begin(), this->end());
  }

  [[nodiscard]] constexpr auto containsInvalidChar() const -> bool {
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

  [[nodiscard]] constexpr auto hasValidNameLength() const -> bool {
    if (this->shortName == '\0') {
      return true;
    }
    return (N - this->nameLen) == 2;
  }
};

template <std::size_t N>
ArgName(const char (&)[N]) -> ArgName<N - 1>;

template <typename BaseType, ArgName Name, bool Required, ParserID ID>
struct ArgBase {
  static constexpr auto name = Name;
  static constexpr auto id = ID;
  inline static bool assigned = false;
  inline static std::string_view description;
  inline static bool required = Required;
  using baseType = BaseType;
};

template <class T>
concept ArgType = requires(T& x) {
  typename T::baseType;
  typename T::type;

  not std::is_same_v<decltype(T::value), void>;

  std::derived_from<decltype(T::name), ArgNameTag>;
  std::is_same_v<decltype(T::isVariadic), char>;
  std::is_same_v<decltype(T::nargs), NArgs>;
  std::is_same_v<decltype(T::assigned), bool>;
  std::is_same_v<decltype(T::description), std::string_view>;
  std::is_same_v<decltype(T::typeName), std::string>;
  std::is_same_v<decltype(T::required), bool>;
};

struct ArgTag {};

template <class T>
constexpr std::string get_type_name_base_type() {
  if constexpr (std::is_same_v<T, bool>) {
    return "BOOL";
  } else if constexpr (std::is_integral_v<T>) {
    return "NUMBER";
  } else if constexpr (std::is_floating_point_v<T>) {
    return "FLOAT";
  } else if constexpr (std::is_same_v<T, const char*> or
                       std::is_same_v<T, std::string> or
                       std::is_same_v<T, std::string_view>) {
    return "STRING";
  } else {
    return "UNKNOWN";
  }
}

template <class T, NArgs TNArgs>
constexpr std::string get_type_name() {
  if constexpr (is_array_v<T> or TNArgs.nargs > 1) {
    std::string ret("<");
    auto base_type_name = std::string();
    if constexpr (is_array_v<T>) {
      base_type_name = get_type_name_base_type<array_base_t<T>>();
    } else if constexpr (is_vector_v<T>) {
      base_type_name = get_type_name_base_type<vector_base_t<T>>();
    } else if constexpr (is_tuple_v<T>) {
      return std::string("<") +
             []<std::size_t... Is>(std::index_sequence<Is...>) {
               std::string ret =
                   ((get_type_name_base_type<std::tuple_element_t<Is, T>>() +
                     std::string(",")) +
                    ...);
               ret.pop_back();
               return ret;
             }(std::make_index_sequence<std::tuple_size_v<T>>()) +
             std::string(">");
    } else {
      throw std::runtime_error("Error");
    }
    for (std::size_t i = 0; i < TNArgs.nargs; i++) {
      ret += base_type_name;
      ret.push_back(',');
    }
    ret.pop_back();
    ret.push_back('>');
    return ret;
  } else if constexpr (TNArgs.nargs == 1) {
    if constexpr (is_vector_v<T>) {
      return get_type_name_base_type<vector_base_t<T>>();
    } else {
      return get_type_name_base_type<T>();
    }
  } else if constexpr (is_vector_v<T>) {
    if constexpr (TNArgs.nargs_char == '*') {
      return std::string("[<") + get_type_name_base_type<vector_base_t<T>>() +
             ",...>]";
    } else if constexpr (TNArgs.nargs_char == '+') {
      return std::string("<") + get_type_name_base_type<vector_base_t<T>>() +
             ",...>";
    }
  } else {
    if constexpr (TNArgs.nargs_char == '?') {
      return std::string("[<") + get_type_name_base_type<T>() +
             std::string(">]");
    }
  }
}

/*!
 * Arg type this holds argument value
 */
template <class Type, ArgName Name, NArgs TNArgs, bool Required, ParserID ID>
struct Arg : ArgTag,
             ArgBase<                          //
                 std::conditional_t<           //
                     is_array_v<Type>,         //
                     array_base_t<Type>,       //
                     std::conditional_t<       //
                         is_vector_v<Type>,    //
                         vector_base_t<Type>,  //
                         Type                  //
                         >                     //
                     >,                        //
                 Name,                         //
                 Required,                     //
                 ID                            //
                 > {
  static constexpr bool isVariadic = (TNArgs.nargs > 1) ||
                                     (TNArgs.nargs_char == '+') ||
                                     (TNArgs.nargs_char == '*');
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
  inline static std::function<type(std::string_view)> caster = nullptr;
  inline static std::function<void(
      const type& value, std::span<std::string_view>, std::string_view)>
      validator = nullptr;
  inline static std::function<void(type&, std::span<std::string_view>)>
      callback = nullptr;
  inline static std::string typeName = get_type_name<type, TNArgs>();
  inline static bool required = Required;
};

struct FlagArgTag {};

template <ArgName Name, ParserID ID>
struct FlagArg : FlagArgTag, ArgBase<bool, Name, false, ID> {
  static constexpr bool isVariadic = false;
  using type = bool;

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = NArgs(-1);
  inline static std::function<void()> callback = nullptr;
  inline static std::string typeName;
};

struct HelpArgTag {};

template <ArgName Name, ParserID ID>
struct HelpArg : HelpArgTag, FlagArgTag, ArgBase<bool, Name, true, ID> {
  static constexpr bool isVariadic = false;
  using type = bool;
  inline static type value = {};
  inline static type defaultValue = {};

  inline static bool assigned = false;
  inline static std::string_view description = "Print help information";
  inline static bool required = false;

  inline static constexpr NArgs nargs = NArgs(-1);
  inline static std::function<void()> callback = nullptr;
  inline static std::string typeName;
};

}  // namespace Argo

// generator end here
