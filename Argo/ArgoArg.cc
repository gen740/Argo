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
  constexpr ParserID(int id) : id(id){};

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr ParserID(const char (&id)[N + 1]) {
    for (size_t i = 0; i < N; i++) {
      this->id.idName[i] = id[i];
    }
  };
};

export ParserID(int) -> ParserID<0>;

export template <size_t N>
ParserID(const char (&)[N]) -> ParserID<N - 1>;

/*!
 * (default)?  : If value specified use it else use default -> ValueType
 *          int: Exactly (n > 1)                     -> array<ValueType, N>
 *          *  : Any number of argument if zero use default -> vector<ValueType>
 *          +  : Any number of argument except zero         -> vector<ValueType>
 */
export struct NArgs {
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

export template <class T>
concept ArgType = requires(T& x) {
  typename T::baseType;
  typename T::type;

  not is_same_v<decltype(T::value), void>;

  derived_from<decltype(T::name), ArgNameTag>;
  is_same_v<decltype(T::nargs), NArgs>;
  is_same_v<decltype(T::assigned), bool>;
  is_same_v<decltype(T::description), string_view>;
  is_same_v<decltype(T::typeName), string>;
  is_same_v<decltype(T::required), bool>;
};

export struct ArgTag {};

template <size_t N>
struct constexprString {};

template <class T>
constexpr string get_type_name_base_type() {
  if constexpr (is_same_v<T, bool>) {
    return "BOOL";
  } else if constexpr (is_integral_v<T>) {
    return "NUMBER";
  } else if constexpr (is_floating_point_v<T>) {
    return "FLOAT";
  } else if constexpr (is_same_v<T, const char*> or is_same_v<T, string> or
                       is_same_v<T, string_view>) {
    return "STRING";
  } else {
    return "UNKNOWN";
  }
}

template <class T, NArgs TNArgs>
constexpr string get_type_name() {
  if constexpr (is_array_v<T> or TNArgs.nargs > 1) {
    string ret("<");
    auto base_type_name = string();
    if constexpr (is_array_v<T>) {
      base_type_name = get_type_name_base_type<array_base_t<T>>();
    } else if constexpr (is_vector_v<T>) {
      base_type_name = get_type_name_base_type<vector_base_t<T>>();
    } else if constexpr (is_tuple_v<T>) {
      return string("<") +
             []<size_t... Is>(index_sequence<Is...>) {
               string ret =
                   ((get_type_name_base_type<tuple_element_t<Is, T>>() +
                     string(",")) +
                    ...);
               ret.pop_back();
               return ret;
             }(make_index_sequence<tuple_size_v<T>>()) +
             string(">");
    } else {
      throw runtime_error("Error");
    }
    for (size_t i = 0; i < TNArgs.nargs; i++) {
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
      return string("[<") + get_type_name_base_type<vector_base_t<T>>() +
             ",...>]";
    } else if constexpr (TNArgs.nargs_char == '+') {
      return string("<") + get_type_name_base_type<vector_base_t<T>>() +
             ",...>";
    }
  } else {
    if constexpr (TNArgs.nargs_char == '?') {
      return string("[<") + get_type_name_base_type<T>() + string(">]");
    }
  }
}

/*!
 * Arg type this holds argument value
 */
export template <class Type, ArgName Name, NArgs TNArgs, bool Required,
                 ParserID ID>
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
  inline static string typeName = get_type_name<type, TNArgs>();
  inline static bool required = Required;
};

export struct FlagArgTag {};

export template <ArgName Name, ParserID ID>
struct FlagArg : FlagArgTag, ArgBase<bool, Name, false, ID> {
  using type = bool;

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = NArgs(-1);
  inline static function<void()> callback = nullptr;
  inline static string typeName;
};

export struct HelpArgTag {};

export template <ArgName Name, ParserID ID>
struct HelpArg : HelpArgTag, FlagArgTag, ArgBase<bool, Name, false, ID> {
  using type = bool;
  inline static type value = {};
  inline static type defaultValue = {};

  inline static string_view description = "Print help information";

  inline static constexpr NArgs nargs = NArgs(-1);
  inline static function<void()> callback = nullptr;
  inline static string typeName;
};

}  // namespace Argo

// generator end here
