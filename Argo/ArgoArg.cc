export module Argo:Arg;

import :ArgName;
import :Validation;
import :TypeTraits;
import :std_module;

// generator start here

namespace Argo {

/*!
 * ParserID which holds parser id
 * You can set int or string as id
 * Example:
 *      // int id
 *      auto parser = Parser<42>();
 *      // string id
 *      auto parser = Parser<"ID">();
 */
template <std::size_t N>
struct ParserID {
  union {
    int idInt = 0;
    char idName[N];
  } id;

  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval ParserID(int id) : id(id){};

  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval ParserID(const char (&id)[N + 1]) {
    for (std::size_t i = 0; i < N; i++) {
      this->id.idName[i] = id[i];
    }
  };
};

ParserID(int) -> ParserID<1>;

template <std::size_t N>
ParserID(const char (&)[N]) -> ParserID<N - 1>;

/*!
 * NArgs which holds number of argument
 * Available argument:
 *   ?  : If value specified use it else use default -> ValueType
 *   int: Exactly (n > 1)                            -> array<ValueType, N>
 *   *  : Any number of argument if zero use default -> vector<ValueType>
 *   +  : Any number of argument except zero         -> vector<ValueType>
 *
 * Note:
 *   default value is "?".
 */
struct NArgs {
  int nargs = -1;
  char nargs_char = '\0';

  constexpr explicit NArgs(char arg) : nargs_char(arg) {}

  constexpr explicit NArgs(int arg) : nargs(arg) {}
};

/*!
 * consteval String
 */
template <std::size_t N>
struct String {
  char str_[N] = {};

  String() = default;

  consteval explicit String(const char (&str)[N + 1]) {
    for (std::size_t i = 0; i < N; i++) {
      str_[i] = str[i];
    }
  };

  consteval explicit operator std::string() const {
    return std::string(str_, N);
  }

  consteval explicit operator std::string_view() const {
    return std::string_view(str_, N);
  }

  [[nodiscard]] consteval auto operator[](std::size_t n) const {
    return str_[n];
  }

  consteval auto operator[](std::size_t n) -> char& {
    return str_[n];
  }

  consteval auto removeTrail() {
    String<N - 1> ret;
    for (std::size_t i = 0; i < N - 1; i++) {
      ret[i] = str_[i];
    }
    return ret;
  }

  template <std::size_t M>
  consteval auto operator+(const String<M>& rhs) -> String<N + M> {
    String<N + M> ret;
    for (std::size_t i = 0; i < N; i++) {
      ret[i] = str_[i];
    }
    for (std::size_t i = 0; i < M; i++) {
      ret[i + N] = rhs[i];
    }
    return ret;
  }
};

template <std::size_t N>
String(const char (&)[N]) -> String<N - 1>;

/*!
 * Convert typename to consteval String
 */
template <class T>
consteval auto get_type_name_base_type([[maybe_unused]] std::size_t n = 0) {
  if constexpr (std::is_same_v<T, bool>) {
    return String("BOOL");
  } else if constexpr (std::is_integral_v<T>) {
    return String("NUMBER");
  } else if constexpr (std::is_floating_point_v<T>) {
    return String("FLOAT");
  } else if constexpr (std::is_same_v<T, const char*> or
                       std::is_same_v<T, std::string> or
                       std::is_same_v<T, std::string_view>) {
    return String("STRING");
  } else if constexpr (std::is_same_v<T, std::filesystem::path>) {
    return String("PATH");
  } else {
    return String("UNKNOWN");
  }
}

template <class T, NArgs TNArgs>
consteval auto get_base_type_name_form_stl() {
  if constexpr (is_array_v<T>) {
    return []<std::size_t... Is>(std::index_sequence<Is...>) consteval {
      return ((get_type_name_base_type<array_base_t<T>>(Is) + String(",")) +
              ...);
    }(std::make_index_sequence<TNArgs.nargs>())
               .removeTrail();
  } else if constexpr (is_vector_v<T>) {
    return []<std::size_t... Is>(std::index_sequence<Is...>) consteval {
      return ((get_type_name_base_type<vector_base_t<T>>(Is) + String(",")) +
              ...)
          .removeTrail();
    }(std::make_index_sequence<TNArgs.nargs>());
  } else if constexpr (is_tuple_v<T>) {
    return []<class... U>(type_sequence<U...>) consteval {
      return (((get_type_name_base_type<vector_base_t<U>>()) + String(",")) +
              ...);
    }(make_type_sequence_t<T>())
               .removeTrail();
  } else {
    return String("UNKNOWN");
  }
}

template <class T, NArgs TNArgs>
consteval auto get_type_name() {
  if constexpr (is_array_v<T> or TNArgs.nargs > 1) {
    return String("<") + get_base_type_name_form_stl<T, TNArgs>() + String(">");
  } else if constexpr (TNArgs.nargs == 1) {
    if constexpr (is_vector_v<T>) {
      return get_type_name_base_type<vector_base_t<T>>();
    } else {
      return get_type_name_base_type<T>();
    }
  } else if constexpr (is_vector_v<T>) {
    if constexpr (TNArgs.nargs_char == '*') {
      return String("[<") + get_type_name_base_type<vector_base_t<T>>() +
             String(",...>]");
    } else if constexpr (TNArgs.nargs_char == '+') {
      return String("<") + get_type_name_base_type<vector_base_t<T>>() +
             String(",...>");
    }
  } else {
    if constexpr (TNArgs.nargs_char == '?') {
      return String("[<") + get_type_name_base_type<T>() + String(">]");
    }
  }
}

struct ArgTag {};

/*!
 * Arg type this holds argument value
 */
template <class Type, ArgName Name, NArgs TNArgs, bool Required, ParserID ID>
struct Arg : ArgTag {
  using type =             //
      std::conditional_t<  //
          ((TNArgs.nargs <= 1) && (TNArgs.nargs_char != '+') &&
           (TNArgs.nargs_char != '*'))                              //
              || is_array_v<Type>                                   //
              || is_tuple_v<Type>                                   //
              || is_vector_v<Type>,                                 //
          Type,                                                     //
          std::conditional_t<                                       //
              (TNArgs.nargs > 1),                                   //
              std::array<Type, static_cast<std::size_t>(TNArgs.nargs)>,  //
              std::vector<Type>                                     //
              >                                                     //
          >;                                                        //
  using baseType = std::conditional_t<                              //
      is_array_v<Type>,                                             //
      array_base_t<Type>,                                           //
      std::conditional_t<                                           //
          is_vector_v<Type>,                                        //
          vector_base_t<Type>,                                      //
          Type                                                      //
          >                                                         //
      >;
  static constexpr auto name = Name;
  inline static std::string_view description{};
  inline static bool assigned = false;
  inline static bool required = Required;
  inline static type value = {};
  inline static type defaultValue = {};
  inline static constexpr NArgs nargs = TNArgs;
  inline static std::function<type(std::string_view)> caster = nullptr;
  inline static std::function<void(
      const type& value, std::span<std::string_view>, std::string_view)>
      validator = nullptr;
  inline static std::function<void(type&, std::span<std::string_view>)>
      callback = nullptr;
  inline static constexpr auto typeName = get_type_name<type, TNArgs>();
};

struct FlagArgTag {};

template <ArgName Name, ParserID ID>
struct FlagArg : FlagArgTag {
  using type = bool;
  using baseType = bool;
  static constexpr auto name = Name;
  inline static bool assigned = false;
  inline static std::string_view description{};
  inline static type value = false;
  inline static std::function<void()> callback = nullptr;
  inline static constexpr auto typeName = String("");
};

struct HelpArgTag {};

template <ArgName Name, ParserID ID>
struct HelpArg : HelpArgTag, FlagArgTag {
  using type = bool;
  using baseType = bool;
  static constexpr auto name = Name;
  inline static bool assigned = false;
  inline static std::string_view description = "Print help information";
  inline static type value = false;
  inline static std::function<void()> callback = nullptr;
  inline static constexpr auto typeName = String("");
};

}  // namespace Argo

// generator end here
