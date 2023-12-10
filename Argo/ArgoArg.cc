module;

export module Argo:Arg;

import :ArgName;
import :Validation;
import :TypeTraits;
import :std_module;

// generator start here

namespace Argo {

using namespace std;

export template <size_t N>
struct ParserID {
  union {
    int idInt = 0;
    char idName[N];
  } id;

  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval ParserID(int id) : id(id){};

  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval ParserID(const char (&id)[N + 1]) {
    for (size_t i = 0; i < N; i++) {
      this->id.idName[i] = id[i];
    }
  };
};

export ParserID(int) -> ParserID<1>;

export template <size_t N>
ParserID(const char (&)[N]) -> ParserID<N - 1>;

/*!
 * (default)?  : If value specified use it else use default -> ValueType
 *          int: Exactly (n > 1)                     -> array<ValueType, N>
 *          *  : Any number of argument if zero use default -> vector<ValueType>
 *          +  : Any number of argument except zero         -> vector<ValueType>
 */
struct NArgs {
  int nargs = -1;
  char nargs_char = '\0';

  constexpr explicit NArgs(char arg) : nargs_char(arg) {}

  constexpr explicit NArgs(int arg) : nargs(arg) {}

  [[nodiscard]] constexpr int getNargs() const {
    return nargs;
  }

  [[nodiscard]] constexpr char getNargsChar() const {
    return nargs_char;
  }
};

template <typename BaseType, ArgName Name, bool Required, ParserID ID>
struct ArgBase {
  static constexpr auto name = Name;
  static constexpr auto id = ID;
  inline static bool assigned = false;
  inline static string_view description;
  inline static bool required = Required;
  using baseType = BaseType;
};

template <size_t N>
struct String {
  char str_[N] = {};

  String() = default;

  consteval explicit String(const char (&str)[N + 1]) {
    for (size_t i = 0; i < N; i++) {
      str_[i] = str[i];
    }
  };

  constexpr explicit operator string() {
    return string(str_, N);
  }

  constexpr explicit operator string_view() {
    return string_view(str_, N);
  }

  [[nodiscard]] consteval auto operator[](size_t n) const {
    return str_[n];
  }

  consteval auto operator[](size_t n) -> char& {
    return str_[n];
  }

  consteval auto removeTrail() {
    String<N - 1> ret;
    for (size_t i = 0; i < N - 1; i++) {
      ret[i] = str_[i];
    }
    return ret;
  }

  template <size_t M>
  consteval auto operator+(const String<M>& rhs) -> String<N + M> {
    String<N + M> ret;
    for (size_t i = 0; i < N; i++) {
      ret[i] = str_[i];
    }
    for (size_t i = 0; i < M; i++) {
      ret[i + N] = rhs[i];
    }
    return ret;
  }
};

template <size_t N>
String(const char (&)[N]) -> String<N - 1>;

template <class T>
consteval auto get_type_name_base_type([[maybe_unused]] size_t n = 0) {
  if constexpr (is_same_v<T, bool>) {
    return String("BOOL");
  } else if constexpr (is_integral_v<T>) {
    return String("NUMBER");
  } else if constexpr (is_floating_point_v<T>) {
    return String("FLOAT");
  } else if constexpr (is_same_v<T, const char*> or is_same_v<T, string> or
                       is_same_v<T, string_view>) {
    return String("STRING");
  } else {
    return String("UNKNOWN");
  }
}

template <class T, NArgs TNArgs>
consteval auto get_base_type_name_form_stl() {
  if constexpr (is_array_v<T>) {
    return []<size_t... Is>(index_sequence<Is...>) {
      return ((get_type_name_base_type<array_base_t<T>>(Is) + String(",")) +
              ...);
    }(make_index_sequence<TNArgs.getNargs()>())
               .removeTrail();
  } else if constexpr (is_vector_v<T>) {
    return []<size_t... Is>(index_sequence<Is...>) {
      return ((get_type_name_base_type<vector_base_t<T>>(Is) + String(",")) +
              ...)
          .removeTrail();
    }(make_index_sequence<TNArgs.getNargs()>());
  } else if constexpr (is_tuple_v<T>) {
    return []<class... U>(type_sequence<U...>) {
      return (((get_type_name_base_type<vector_base_t<U>>()) + String(",")) +
              ...);
    }(make_type_sequence_t<T>())
               .removeTrail();
  } else {
    return String("UNKNOWN");
  }
}

template <class T, NArgs TNArgs>
constexpr auto get_type_name() {
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
struct Arg : ArgTag,
             ArgBase<                          //
                 conditional_t<                //
                     is_array_v<Type>,         //
                     array_base_t<Type>,       //
                     conditional_t<            //
                         is_vector_v<Type>,    //
                         vector_base_t<Type>,  //
                         Type                  //
                         >                     //
                     >,                        //
                 Name,                         //
                 Required,                     //
                 ID                            //
                 > {
  using type =        //
      conditional_t<  //
          ((TNArgs.nargs <= 1) && (TNArgs.nargs_char != '+') &&
           (TNArgs.nargs_char != '*'))                         //
              || is_array_v<Type>                              //
              || is_tuple_v<Type>                              //
              || is_vector_v<Type>,                            //
          Type,                                                //
          conditional_t<                                       //
              (TNArgs.nargs > 1),                              //
              array<Type, static_cast<size_t>(TNArgs.nargs)>,  //
              vector<Type>                                     //
              >                                                //
          >;                                                   //

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = TNArgs;
  inline static function<type(string_view)> caster = nullptr;
  inline static function<void(const type& value, span<string_view>,
                              string_view)>
      validator = nullptr;
  inline static function<void(type&, span<string_view>)> callback = nullptr;
  inline static auto typeName = get_type_name<type, TNArgs>();
  inline static bool required = Required;
};

struct FlagArgTag {};

template <ArgName Name, ParserID ID>
struct FlagArg : FlagArgTag, ArgBase<bool, Name, false, ID> {
  using type = bool;

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = NArgs(-1);
  inline static function<void()> callback = nullptr;
  inline static auto typeName = String("");
};

struct HelpArgTag {};

template <ArgName Name, ParserID ID>
struct HelpArg : HelpArgTag, FlagArgTag, ArgBase<bool, Name, false, ID> {
  using type = bool;
  inline static type value = {};
  inline static type defaultValue = {};

  inline static string_view description = "Print help information";

  inline static constexpr NArgs nargs = NArgs(-1);
  inline static function<void()> callback = nullptr;
  inline static auto typeName = String("");
};

template <class T>
concept ArgType = requires(T& x) {
  typename T::baseType;
  typename T::type;

  not is_same_v<decltype(T::value), void>;

  { T::name } -> ArgNameType;
  is_same_v<decltype(T::nargs), NArgs>;
  is_same_v<decltype(T::assigned), bool>;
  is_same_v<decltype(T::description), string_view>;
  is_convertible_v<decltype(T::typeName), string>;
  is_convertible_v<decltype(T::typeName), string_view>;
  is_same_v<decltype(T::required), bool>;
};

}  // namespace Argo

// generator end here
