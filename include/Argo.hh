#pragma once

#include <unistd.h>

#include <array>
#include <charconv>
#include <cassert>
#include <concepts>
#include <cstring>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <print>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>


namespace Argo {

using namespace std;

class ParserInternalError : public runtime_error {
 public:
  explicit ParserInternalError(const string& msg) : runtime_error(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return runtime_error::what();
  }
};

class ParseError : public runtime_error {
 public:
  explicit ParseError(const string& msg) : runtime_error(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return runtime_error::what();
  }
};

/*!
 * InvalidArgument exception class
 */
class InvalidArgument : public invalid_argument {
 public:
  explicit InvalidArgument(const string& msg) : invalid_argument(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return invalid_argument::what();
  }
};

class ValidationError : public InvalidArgument {
 public:
  explicit ValidationError(const string& msg) : InvalidArgument(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return InvalidArgument::what();
  }
};

}  // namespace Argo


namespace Argo {

using namespace std;

template <class T>
struct is_tuple : false_type {};

template <class... T>
struct is_tuple<tuple<T...>> : true_type {};

template <class T>
constexpr bool is_tuple_v = is_tuple<T>::value;

template <class T>
struct is_vector : false_type {};

template <class T>
struct is_vector<vector<T>> : true_type {};

template <class T>
constexpr bool is_vector_v = is_vector<T>::value;

template <class T>
struct vector_base {
  using type = T;
};

template <class T>
struct vector_base<vector<T>> {
  using type = T;
};

template <class T>
using vector_base_t = vector_base<T>::type;

template <class T>
struct is_array : false_type {};

template <class T, size_t N>
struct is_array<array<T, N>> : true_type {};

template <class T>
constexpr bool is_array_v = is_array<T>::value;

template <class T>
struct array_len {
  static constexpr size_t value = 0;
};

template <class T, size_t N>
struct array_len<array<T, N>> {
  static constexpr size_t value = N;
};

template <class T>
constexpr size_t array_len_v = array_len<T>::value;

template <class T>
struct array_base {
  using type = T;
};

template <class T, size_t N>
struct array_base<array<T, N>> {
  using type = T;
};

template <class T>
using array_base_t = array_base<T>::type;

template <class T, class... U>
struct tuple_append {
  using type = tuple<U..., T>;
};

template <class T, class... U>
struct tuple_append<tuple<U...>, T> {
  using type = tuple<U..., T>;
};

template <class... T>
using tuple_append_t = typename tuple_append<T...>::type;

};  // namespace Argo


namespace Argo::Validation {

using namespace std;

struct ValidationBase {
  template <class T>
  auto operator()(const T& value, span<string_view> values,
                  string_view option_name) -> void {
    if (!this->isValid(value, values)) {
      throw ValidationError(
          format("Option {} has invalid value {}", option_name, value));
    }
  }

  template <class T>
  auto operator()(const T& value, span<string_view> raw_val) {
    return this->isValid(value, raw_val);
  }

  template <class T>
  auto isValid(const T& /* unused */, span<string_view> /* unuesd */) const
      -> bool {
    static_assert(false, "Invalid validation");
  };

  virtual ~ValidationBase() = default;
};

template <derived_from<ValidationBase> Lhs, derived_from<ValidationBase> Rhs>
struct AndValidation : ValidationBase {
 private:
  Lhs lhs_;
  Rhs rhs_;

 public:
  AndValidation(Lhs lhs, Rhs rhs) : lhs_(lhs), rhs_(rhs){};

  template <class U>
  auto isValid(const U& value, span<string_view> raw_values) const -> bool {
    return this->lhs_(value, raw_values) && this->lhs_(value, raw_values);
  };
};

template <derived_from<ValidationBase> Lhs, derived_from<ValidationBase> Rhs>
AndValidation(Lhs, Rhs) -> AndValidation<Lhs, Rhs>;

template <derived_from<ValidationBase> Lhs, derived_from<ValidationBase> Rhs>
struct OrValidation : ValidationBase {
 private:
  Lhs lhs_;
  Rhs rhs_;

 public:
  OrValidation(Lhs lhs, Rhs rhs) : lhs_(lhs), rhs_(rhs){};

  template <class U>
  auto isValid(const U& value, span<string_view> raw_values) const -> bool {
    return this->lhs_(value, raw_values) || this->lhs_(value, raw_values);
  };
};

template <derived_from<ValidationBase> Lhs, derived_from<ValidationBase> Rhs>
OrValidation(Lhs, Rhs) -> OrValidation<Lhs, Rhs>;

template <derived_from<ValidationBase> Rhs>
struct InvertValidation : ValidationBase {
 private:
  Rhs rhs_;

 public:
  explicit InvertValidation(Rhs rhs) : rhs_(rhs){};

  template <class U>
  auto isValid(const U& value, span<string_view> raw_values) const -> bool {
    return !this->lhs_(value, raw_values);
  };
};

template <derived_from<ValidationBase> Rhs>
InvertValidation(Rhs) -> InvertValidation<Rhs>;

template <class T>
struct Range final : public ValidationBase {
 private:
  T min_;
  T max_;

 public:
  Range(T min, T max) : min_(min), max_(max){};

  template <class U>
  auto operator()(const U& value, span<string_view> values,
                  string_view option_name) -> void {
    if (!this->isValid(value, values)) {
      throw ValidationError(
          format("Option {} has invalid value {}", option_name, value));
    }
  }

  template <class U>
  auto isValid(const U& value, span<string_view> /* unused */) const -> bool {
    return static_cast<U>(this->min_) < value &&
           value < static_cast<U>(this->max_);
  };
};

template <class T>
Range(T min, T max) -> Range<T>;

// template <class Type>
// struct Callback final : public ValidationBase<Type> {
//  private:
//   function<bool(Type)> callback_;
//
//  public:
//   Callback(function<bool(Type)> callback) : callback_(callback){};
//
//   auto isValid(const Type& value) const -> bool {
//     return this->callback_(value);
//   };
// };

}  // namespace Argo::Validation

template <std::derived_from<Argo::Validation::ValidationBase> Lhs,
                 std::derived_from<Argo::Validation::ValidationBase> Rhs>
auto operator&(Lhs lhs, Rhs rhs) {
  return Argo::Validation::AndValidation(lhs, rhs);
}

template <std::derived_from<Argo::Validation::ValidationBase> Lhs,
                 std::derived_from<Argo::Validation::ValidationBase> Rhs>
auto operator|(Lhs lhs, Rhs rhs) {
  return Argo::Validation::OrValidation(lhs, rhs);
}

template <std::derived_from<Argo::Validation::ValidationBase> Lhs,
                 std::derived_from<Argo::Validation::ValidationBase> Rhs>
auto operator!(Rhs rhs) {
  return Argo::Validation::InvertValidation(rhs);
}


namespace Argo {

struct ArgNameTag {};

/*!
 * ArgName which holds argument name
 */
template <size_t N>
struct ArgName : ArgNameTag {
  char name[N] = {};
  char shortName = '\0';
  size_t nameLen = N;

  explicit ArgName() = default;

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr ArgName(const char (&lhs)[N + 1]) {
    for (size_t i = 0; i < N; i++) {
      if (lhs[i] == ',') {
        nameLen = i;
        shortName = lhs[i + 1];
        return;
      }
      this->name[i] = lhs[i];
    }
  };

  constexpr char operator[](size_t idx) const {
    return this->name[idx];
  }

  constexpr char& operator[](size_t idx) {
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

  template <size_t M>
  constexpr auto operator==(ArgName<M> lhs) -> bool {
    if constexpr (M != N) {
      return false;
    } else {
      for (size_t i = 0; i < N; i++) {
        if ((*this)[i] != lhs[i]) {
          return false;
        }
      }
      return true;
    }
  }

  template <size_t M>
  constexpr auto operator==(ArgName<M> lhs) const -> bool {
    auto NV = this->nameLen;
    auto MV = lhs.nameLen;

    if (MV != NV) {
      return false;
    }
    for (size_t i = 0; i < NV; i++) {
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
    for (size_t i = 0; i < this->nameLen; i++) {
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

template <size_t N>
ArgName(const char (&)[N]) -> ArgName<N - 1>;

}  // namespace Argo


namespace Argo {

using namespace std;

template <size_t N>
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

ParserID(int) -> ParserID<0>;

template <size_t N>
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

template <class T>
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

struct ArgTag {};

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
template <class Type, ArgName Name, NArgs TNArgs, bool Required,
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

struct FlagArgTag {};

template <ArgName Name, ParserID ID>
struct FlagArg : FlagArgTag, ArgBase<bool, Name, false, ID> {
  using type = bool;

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = NArgs(-1);
  inline static function<void()> callback = nullptr;
  inline static string typeName;
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
  inline static string typeName;
};

}  // namespace Argo


namespace Argo {

using namespace std;

struct ExplicitDefaultValueTag {};

template <class T>
struct ExlicitDefaultValue : ExplicitDefaultValueTag {
  T explicit_default_value;
};

struct ImplicitDefaultValueTag {};

template <class T>
struct ImplicitDefaultValue : ImplicitDefaultValueTag {
  T implicit_default_value;
};

struct Description {
  string_view description;
};

constexpr auto description(string_view desc) -> Description {
  return {.description = desc};
}

template <class T>
constexpr auto explicitDefault(T value) -> ExlicitDefaultValue<T> {
  return {.explicit_default_value = value};
}

template <class T>
constexpr auto implicitDefault(T value) -> ImplicitDefaultValue<T> {
  return {.implicit_default_value = value};
}

template <class Type, ArgName Name, NArgs nargs, bool Required, ParserID ID>
struct ArgInitializer {
  template <class Head, class... Tails>
  static auto init(Head head, Tails... tails) {
    using Arg = Arg<Type, Name, nargs, Required, ID>;
    if constexpr (is_same_v<Head, Description>) {
      Arg::description = head.description;
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      Validation::ValidationBase>) {
      static_assert(is_invocable_v<decltype(head), typename Arg::type,
                                   span<string_view>, string_view>,
                    "Invalid validator");
      Arg::validator = head;
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      ImplicitDefaultValueTag>) {
      Arg::defaultValue = static_cast<Type>(head.implicit_default_value);
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      ExplicitDefaultValueTag>) {
      Arg::value = static_cast<Type>(head.explicit_default_value);
    } else if constexpr (is_invocable_v<Head, typename Arg::type&,
                                        span<string_view>>) {
      Arg::callback = head;
    } else {
      static_assert(false, "Invalid argument");
    }
    if constexpr (sizeof...(Tails) != 0) {
      init(tails...);
    }
  }

  static auto init() {}
};

template <ArgName Name, ParserID ID>
struct FlagArgInitializer {
  template <class Head, class... Tails>
  static auto init(Head head, Tails... tails) {
    using FlagArg = FlagArg<Name, ID>;
    if constexpr (is_same_v<Head, Description>) {
      FlagArg::description = head.description;
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      Validation::ValidationBase>) {
      static_assert(false, "Flag cannot have validator");
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      ImplicitDefaultValueTag>) {
      static_assert(false, "Flag cannot have implicit default value");
    } else if constexpr (derived_from<remove_cvref_t<Head>,
                                      ExplicitDefaultValueTag>) {
      static_assert(false, "Flag cannot have explicit default value");
    } else if constexpr (is_invocable_v<Head>) {
      FlagArg::callback = head;
    } else {
      static_assert(false, "Invalid argument");
    }
    if constexpr (sizeof...(Tails) != 0) {
      init(tails...);
    }
  }

  static auto init() {}
};

};  // namespace Argo


namespace Argo {

using namespace std;

struct ArgInfo {
  string_view name;
  char shortName;
  string_view description;
  bool required;
  string typeName;
};

template <class Args>
struct HelpGenerator {};

template <class... Args>
struct HelpGenerator<tuple<Args...>> {
  static auto generate() -> vector<ArgInfo> {
    vector<ArgInfo> ret;
    (
        [&ret]<class T>() {
          ret.emplace_back(
              string_view(Args::name).substr(0, Args::name.nameLen),
              Args::name.shortName, Args::description, Args::required,
              Args::typeName);
        }.template operator()<Args>(),
        ...);
    return ret;
  }
};

struct SubCommandInfo {
  string_view name;
  string_view description;
};

template <class T>
auto SubParserInfo(T subparsers) {
  vector<SubCommandInfo> ret{};
  if constexpr (!is_same_v<T, tuple<>>) {
    apply(
        [&ret]<class... Parser>(Parser... parser) {
          (..., ret.emplace_back(parser.name, parser.description));
        },
        subparsers);
  }
  return ret;
};

}  // namespace Argo


namespace Argo {

using namespace std;

template <class Arguments>
struct GetNameFromShortName {
  template <class Head, class... Tails>
  struct GetNameFromShortNameImpl {
    [[noreturn]] static auto get(char /* unused */) -> string_view {
      throw ParserInternalError("Fail to lookup");
    }
  };

  template <class Head, class... Tails>
  struct GetNameFromShortNameImpl<tuple<Head, Tails...>> {
    static auto get(char key) -> string_view {
      auto name = string_view(Head::name);
      if (Head::name.shortName == key) {
        return name;
      }
      return GetNameFromShortNameImpl<tuple<Tails...>>::get(key);
    }
  };

  static auto eval(char key) -> string_view {
    return GetNameFromShortNameImpl<Arguments>::get(key);
  }
};

/*!
 * Index Search meta function
 */
template <class Tuple, ArgName T, int Index = 0>
struct SearchIndex;

template <ArgName T, size_t Index>
struct SearchIndex<tuple<>, T, Index> {
  static constexpr int value = -1;
};

template <ArgName T, size_t Index, class Head, class... Tails>
struct SearchIndex<tuple<Head, Tails...>, T, Index> {
  static constexpr int value =
      (Head::name == T) ? Index
                        : SearchIndex<tuple<Tails...>, T, Index + 1>::value;
};

/*!
 * Index Search meta function
 */
template <class Tuple, char T, int Index = 0>
struct SearchIndexFromShortName;

template <char T, size_t Index>
struct SearchIndexFromShortName<tuple<>, T, Index> {
  static constexpr int value = -1;
};

template <char T, size_t Index, class Head, class... Tails>
struct SearchIndexFromShortName<tuple<Head, Tails...>, T, Index> {
  static constexpr int value =
      Head::name.shortName == T
          ? Index
          : SearchIndexFromShortName<tuple<Tails...>, T, Index + 1>::value;
};

};  // namespace Argo


namespace Argo {

using namespace std;

/*!
 * Helper class of assigning value
 */
template <class Type>
constexpr auto caster(const string_view& value) -> Type {
  if constexpr (is_same_v<Type, bool>) {
    if ((value == "true")     //
        || (value == "True")  //
        || (value == "TRUE")  //
        || (value == "1")) {
      return true;
    }
    if ((value == "false")     //
        || (value == "False")  //
        || (value == "FALSE")  //
        || (value == "0")) {
      return false;
    }
    throw ParserInternalError("Invalid argument expect bool");
  } else if constexpr (is_integral_v<Type>) {
    return static_cast<Type>(stoi(string(value)));
  } else if constexpr (is_floating_point_v<Type>) {
    return static_cast<Type>(stod(string(value)));
  } else if constexpr (is_same_v<Type, const char*>) {
    return value.data();
  } else {
    return static_cast<Type>(value);
  }
}

template <class... T, size_t... N>
constexpr auto tupleAssign(tuple<T...>& t, span<string_view> v,
                           index_sequence<N...> /* unused */) {
  ((get<N>(t) = caster<remove_cvref_t<decltype(get<N>(t))>>(v[N])), ...);
}

template <class PArgs>
struct PArgAssigner {};

template <class... PArgs>
struct PArgAssigner<tuple<PArgs...>> {
  static auto assign(span<string_view> values) {
    return ([]<ArgType Arg>(auto& values) {
      if (Arg::assigned) {
        return false;
      }
      if constexpr (Arg::nargs.getNargsChar() == '+') {
        for (const auto& value : values) {
          Arg::value.push_back(caster<typename Arg::baseType>(value));
        }
        if (Arg::validator) {
          Arg::validator(Arg::value, values, string_view(Arg::name));
        }
        if (Arg::callback) {
          Arg::callback(Arg::value, values);
        }
        Arg::assigned = true;
        return true;
      }
      if constexpr (Arg::nargs.getNargs() > 0) {
        if (Arg::nargs.getNargs() > values.size()) {
          throw Argo::InvalidArgument(
              format("Positional Argument {} Invalid positional argument {}",
                     string_view(Arg::name), values));
        }

        if constexpr (is_array_v<typename Arg::type>) {
          for (int i = 0; i < Arg::nargs.getNargs(); i++) {
            Arg::value[i] = caster<typename Arg::baseType>(values[i]);
          }
        } else if constexpr (is_vector_v<typename Arg::type>) {
          for (int i = 0; i < Arg::nargs.getNargs(); i++) {
            Arg::value.push_back(caster<typename Arg::baseType>(values[i]));
          }
        } else if constexpr (is_tuple_v<typename Arg::type>) {
          tupleAssign(Arg::value, values,
                      make_index_sequence<tuple_size_v<typename Arg::type>>());
        } else {
          static_assert(false, "Invalid Type");
        }

        if (Arg::validator) {
          Arg::validator(Arg::value, values.subspan(0, Arg::nargs.getNargs()),
                         string_view(Arg::name));
        }
        if (Arg::callback) {
          Arg::callback(Arg::value, values.subspan(0, Arg::nargs.getNargs()));
        }
        Arg::assigned = true;
        values = values.subspan(Arg::nargs.getNargs());
        return values.empty();
      }
    }.template operator()<PArgs>(values) ||
            ...);
  }
};

template <class Arguments, class PArgs>
struct Assigner {
  template <ArgType Head>
  static constexpr auto assignOneArg(const string_view& key,
                                     const span<string_view>& values) -> bool {
    if constexpr (derived_from<Head, FlagArgTag>) {
      if (!values.empty()) {
        if constexpr (is_same_v<PArgs, tuple<>>) {
          throw Argo::InvalidArgument(
              format("Flag {} can not take value", key));
        } else {
          PArgAssigner<PArgs>::assign(values);
        }
      }
      Head::value = true;
      Head::assigned = true;
      if (Head::callback) {
        Head::callback();
      }
      return true;
    } else {
      if constexpr (Head::nargs.getNargsChar() == '?') {
        if (values.empty()) {
          Head::value = Head::defaultValue;
          Head::assigned = true;
          return true;
        }
        if (values.size() == 1) {
          Head::value = caster<typename Head::type>(values[0]);
          Head::assigned = true;
          if (Head::validator) {
            Head::validator(Head::value, values, key);
          }
          if (Head::callback) {
            Head::callback(Head::value, values);
          }
          return true;
        }
        if constexpr (is_same_v<PArgs, tuple<>>) {
          throw Argo::InvalidArgument(
              format("Argument {} cannot take more than one value got {}", key,
                     values.size()));
        } else {
          assignOneArg<Head>(key, values.subspan(0, 1));
          return PArgAssigner<PArgs>::assign(values.subspan(1));
        }
      } else if constexpr (Head::nargs.getNargsChar() == '*') {
        if (values.empty()) {
          Head::value = Head::defaultValue;
          Head::assigned = true;
          return true;
        }
        for (const auto& value : values) {
          Head::value.emplace_back(
              caster<vector_base_t<typename Head::type>>(value));
        }
        Head::assigned = true;
        if (Head::validator) {
          Head::validator(Head::value, values, key);
        }
        if (Head::callback) {
          Head::callback(Head::value, values);
        }
        return true;
      } else if constexpr (Head::nargs.getNargsChar() == '+') {
        if (values.empty()) {
          throw Argo::InvalidArgument(
              format("Argument {} should take more than one value", key));
        }
        for (const auto& value : values) {
          Head::value.emplace_back(caster<typename Head::baseType>(value));
        }
        Head::assigned = true;
        if (Head::validator) {
          Head::validator(Head::value, values, key);
        }
        if (Head::callback) {
          Head::callback(Head::value, values);
        }
        return true;
      } else if constexpr (Head::nargs.getNargs() == 1) {
        if (values.empty()) {
          throw Argo::InvalidArgument(format(
              "Argument {} should take exactly one value but zero", key));
        }
        if (values.size() > 1) {
          if constexpr (is_same_v<PArgs, tuple<>>) {
            throw Argo::InvalidArgument(
                format("Argument {} should take exactly one value but {}", key,
                       values.size()));
          } else {
            assignOneArg<Head>(key, values.subspan(0, 1));
            return PArgAssigner<PArgs>::assign(values.subspan(1));
          }
        }
        Head::value = caster<typename Head::baseType>(values[0]);
        Head::assigned = true;
        if (Head::validator) {
          Head::validator(Head::value, values, key);
        }
        if (Head::callback) {
          Head::callback(Head::value, values);
        }
        return true;
      } else {
        if (values.size() == Head::nargs.getNargs()) {
          if constexpr (is_array_v<typename Head::type>) {
            for (int idx = 0; idx < Head::nargs.getNargs(); idx++) {
              Head::value[idx] =
                  caster<array_base_t<typename Head::type>>(values[idx]);
            }
          } else if constexpr (is_tuple_v<typename Head::type>) {
            tupleAssign(
                Head::value, values,
                make_index_sequence<tuple_size_v<typename Head::type>>());
          } else {
            for (const auto& value : values) {
              Head::value.emplace_back(
                  caster<vector_base_t<typename Head::type>>(value));
            }
          }

          Head::assigned = true;
          if (Head::validator) {
            Head::validator(Head::value, values, key);
          }
          if (Head::callback) {
            Head::callback(Head::value, values);
          }
          return true;
        }
        if (values.size() < Head::nargs.getNargs()) {
          throw Argo::InvalidArgument(
              format("Argument {} should take exactly {} value but {}", key,
                     Head::nargs.getNargs(), values.size()));
        }
        if constexpr (is_same_v<PArgs, tuple<>>) {
          throw Argo::InvalidArgument(
              format("Argument {} should take exactly {} value but {}", key,
                     Head::nargs.getNargs(), values.size()));
        } else {
          assignOneArg<Head>(key, values.subspan(0, Head::nargs.getNargs()));
          return PArgAssigner<PArgs>::assign(
              values.subspan(Head::nargs.getNargs()));
        }
      }
    }
    return false;
  }

  template <class Args>
  static auto assignImpl([[maybe_unused]] const string_view& key,
                         [[maybe_unused]] const span<string_view>& values) {
    [&key, &values]<size_t... Is>(index_sequence<Is...> /*unused*/) {
      if (!(... || (string_view(tuple_element_t<Is, Args>::name) == key and
                    assignOneArg<tuple_element_t<Is, Args>>(key, values)))) {
        throw Argo::InvalidArgument(format("Invalid argument {}", key));
      }
    }(make_index_sequence<tuple_size_v<Args>>());
  }

  template <class T>
  static auto assignFlagImpl(string_view key) -> void {
    [&key]<size_t... Is>(index_sequence<Is...>) {
      if (!(... || [&key]<ArgType Head>() {
            if constexpr (derived_from<Head, FlagArgTag>) {
              if (string_view(Head::name) == key) {
                Head::value = true;
                return true;
              }
              return false;
            } else {
              return false;
            }
          }.template operator()<tuple_element_t<Is, T>>())) {
        throw Argo::InvalidArgument("");
      }
    }(make_index_sequence<tuple_size_v<T>>());
  }

  static auto assign(string_view key, span<string_view> values) -> void {
    if (key.empty()) {
      if constexpr (!is_same_v<PArgs, tuple<>>) {
        if (!PArgAssigner<PArgs>::assign(values)) {
          throw InvalidArgument(format("Duplicated positional argument"));
        }
        return;
      } else {
        throw Argo::InvalidArgument(
            format("Assigner: Invalid argument {}", key));
      }
    }
    assignImpl<Arguments>(key, values);
  };

  // Multiple key assigner

  static auto assign(span<char> key, span<string_view> values) {
    for (size_t i = 0; i < key.size() - 1; i++) {
      assignFlagImpl<Arguments>(GetNameFromShortName<Arguments>::eval(key[i]));
    }
    assignImpl<Arguments>(GetNameFromShortName<Arguments>::eval(key.back()),
                          values);
  };
};

template <class Args>
auto ValueReset() {
  []<size_t... Is>(index_sequence<Is...>) {
    (..., []<ArgType T>() {
      if (T::assigned) {
        T::value = typename T::type();
        T::assigned = false;
      }
    }.template operator()<tuple_element_t<Is, Args>>());
  }(make_index_sequence<tuple_size_v<Args>>());
}

};  // namespace Argo


namespace Argo {

using namespace std;

/*!
 * Checking if the given argument is required key, cycle through all the
 * tuple argument.
 */
template <class Args>
struct RequiredChecker {};

template <ArgType... Args>
struct RequiredChecker<tuple<Args...>> {
  static auto check() -> vector<string_view> {
    auto required_keys = vector<string_view>();
    (
        [&required_keys]<class T>() {
          if constexpr (derived_from<T, ArgTag>) {
            if (T::required && !T::assigned) {
              required_keys.push_back(string_view(T::name));
            }
          }
        }.template operator()<Args>(),
        ...);
    return required_keys;
  };
};

/*!
 * Checking if the given argument is assigned, cycle through all the
 * tuple argument.
 */
template <class Args>
struct AssignChecker {};

template <ArgType... Args>
struct AssignChecker<tuple<Args...>> {
  static auto check() -> vector<string_view> {
    auto assigned_keys = vector<string_view>();
    (
        [&assigned_keys]<class T>() {
          if constexpr (derived_from<T, ArgTag>) {
            if (T::assigned) {
              assigned_keys.push_back(string_view(T::name));
            }
          }
        }.template operator()<Args>(),
        ...);
    return assigned_keys;
  };
};

}  // namespace Argo


namespace Argo {

using namespace std;

template <ArgName Name, class Parser>
struct SubParser {
  static constexpr auto name = Name;
  reference_wrapper<Parser> parser;
  string_view description;
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
auto MetaParse(SubParsers sub_parsers, int index, int argc, char** argv)
    -> bool {
  return apply(
      [&](auto&&... s) {
        int64_t idx = -1;
        return (... || (idx++, idx == index &&
                                   (s.parser.get().parse(argc, argv), true)));
      },
      sub_parsers);
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
constexpr auto ParserIndex(SubParsers sub_parsers,  //
                           string_view key) -> int64_t {
  return apply(
      [&](auto&&... s) {
        int64_t index = -1;
        bool found = (... || (index++, s.name == key));
        return found ? index : -1;
      },
      sub_parsers);
};

}  // namespace Argo


namespace Argo {

using namespace std;

struct Unspecified {};

enum class RequiredFlag : bool {
  Optional = false,
  Required = true,
};

using RequiredFlag::Required;
using RequiredFlag::Optional;

/*!
 * Helper function to create nargs
 */
consteval auto nargs(char narg) -> NArgs {
  return NArgs(narg);
}

consteval auto nargs(int narg) -> NArgs {
  return NArgs(narg);
}

struct ParserInfo {
  optional<string_view> help = nullopt;
  optional<string_view> program_name = nullopt;
  optional<string_view> description = nullopt;
  optional<string_view> usage = nullopt;
  optional<string_view> subcommand_help = nullopt;
  optional<string_view> options_help = nullopt;
  optional<string_view> positional_argument_help = nullopt;
};

template <ParserID ID = 0, class Args = tuple<>, class PArgs = tuple<>,
                 class HArg = void, class SubParsers = tuple<>>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
class Parser {
 private:
  bool parsed_ = false;
  unique_ptr<ParserInfo> info_ = nullptr;

 public:
  constexpr explicit Parser() : info_(make_unique<ParserInfo>()){};

  constexpr explicit Parser(string_view program_name)
      : info_(make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
  };

  constexpr explicit Parser(string_view program_name, string_view description)
      : info_(make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
    this->info_->description = description;
  };

  constexpr explicit Parser(string_view program_name,
                            Argo::Description description)
      : info_(make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
    this->info_->description = description.description;
  };

  Parser(const Parser&) = delete;
  Parser(Parser&&) = delete;

  SubParsers subParsers;

  constexpr explicit Parser(SubParsers tuple) : subParsers(tuple) {}

  constexpr explicit Parser(unique_ptr<ParserInfo> info, SubParsers tuple)
      : info_(std::move(info)), subParsers(tuple){};

  template <class Type, ArgName Name, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), bool ISPArgs, class... T>
  auto createArg(T... args) {
    static_assert(!Name.containsInvalidChar(), "Name has invalid char");
    static_assert(Name.hasValidNameLength(),
                  "Short name can't be more than one charactor");

    static constexpr auto nargs = []() {
      if constexpr (is_same_v<remove_cvref_t<decltype(arg1)>, NArgs>) {
        if constexpr (is_array_v<Type>) {
          static_assert(array_len_v<Type> == arg1.nargs,
                        "Array size mismatch with nargs");
        }
        if constexpr (is_vector_v<Type>) {
          static_assert(arg1.nargs_char != '?' && arg1.nargs != 1,
                        "Vector size mismatch with nargs");
        }
        if constexpr (is_tuple_v<Type>) {
          static_assert(tuple_size_v<Type> == arg1.nargs,
                        "Tuple size mismatch with nargs");
        }
        return arg1;
      } else if constexpr (is_same_v<remove_cvref_t<decltype(arg2)>, NArgs>) {
        if constexpr (is_array_v<Type>) {
          static_assert(array_len_v<Type> == arg2.nargs,
                        "Array size mismatch with nargs");
        }
        if constexpr (is_vector_v<Type>) {
          static_assert(arg2.nargs_char != '?' && arg2.nargs != 1,
                        "Vector size mismatch with nargs");
        }
        if constexpr (is_tuple_v<Type>) {
          static_assert(tuple_size_v<Type> == arg2.nargs,
                        "Tuple size mismatch with nargs");
        }
        return arg2;
      } else {
        if constexpr (is_array_v<Type>) {
          return NArgs{static_cast<int>(array_len_v<Type>)};
        }
        if constexpr (is_vector_v<Type>) {
          return NArgs{'*'};
        }
        if constexpr (is_tuple_v<Type>) {
          return NArgs{static_cast<int>(tuple_size_v<Type>)};
        }
        if constexpr (ISPArgs) {
          return NArgs(1);
        }
        return NArgs('?');
      }
    }();

    static_assert(!(is_array_v<Type> and nargs.getNargs() == 1),
                  "Array size must be more than one");
    static_assert(!(is_tuple_v<Type> and nargs.getNargs() == 1),
                  "Tuple size must be more than one");

    static constexpr auto required = []() {
      if constexpr (is_same_v<remove_cvref_t<decltype(arg1)>, RequiredFlag>) {
        return static_cast<bool>(arg1);
      } else if constexpr (is_same_v<remove_cvref_t<decltype(arg2)>,
                                     RequiredFlag>) {
        return static_cast<bool>(arg2);
      } else {
        return false;
      }
    }();
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      static_assert(SearchIndex<PArgs, Name>::value == -1, "Duplicated name");
    }
    static_assert(
        (Name.shortName == '\0') ||
            (SearchIndexFromShortName<Args, Name.shortName>::value == -1),
        "Duplicated short name");
    static_assert(                                   //
        Argo::SearchIndex<Args, Name>::value == -1,  //
        "Duplicated name");
    static_assert(                         //
        (nargs.getNargs() > 0              //
         || nargs.getNargsChar() == '?'    //
         || nargs.getNargsChar() == '+'    //
         || nargs.getNargsChar() == '*'),  //
        "nargs must be '?', '+', '*' or int");

    ArgInitializer<Type, Name, nargs, required, ID>::init(
        std::forward<T>(args)...);
    return type_identity<Arg<Type, Name, nargs, required, ID>>();
  }

  /*!
   * Name: name of argument
   * Type: type of argument
   * arg1: Required(bool) or NArgs or Unspecified
   * arg2: Required(bool) or NArgs or Unspecified
   */
  template <ArgName Name, class Type, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), class... T>
  auto addArg(T... args) {
    auto arg =
        createArg<Type, Name, arg1, arg2, false>(std::forward<T>(args)...);
    return Parser<ID, tuple_append_t<Args, typename decltype(arg)::type>, PArgs,
                  HArg, SubParsers>(std::move(this->info_), subParsers);
  }

  /*!
   * Name: name of argument
   * Type: type of argument
   * arg1: Required(bool) or NArgs or Unspecified
   * arg2: Required(bool) or NArgs or Unspecified
   */
  template <ArgName Name, class Type, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), class... T>
  auto addPositionalArg(T... args) {
    static_assert(Name.shortName == '\0',
                  "Positional argment cannot have short name");
    auto arg =
        createArg<Type, Name, arg1, arg2, true>(std::forward<T>(args)...);

    static_assert(decltype(arg)::type::nargs.getNargsChar() != '?',
                  "Cannot assign narg: ? to the positional argument");
    static_assert(decltype(arg)::type::nargs.getNargsChar() != '*',
                  "Cannot assign narg: * to the positional argument");

    return Parser<ID, Args, tuple_append_t<PArgs, typename decltype(arg)::type>,
                  HArg, SubParsers>(std::move(this->info_), subParsers);
  }

  template <ArgName Name, class... T>
  auto addFlag(T... args) {
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      static_assert(SearchIndex<PArgs, Name>::value == -1, "Duplicated name");
    }
    static_assert(
        (Name.shortName == '\0') ||
            (SearchIndexFromShortName<Args, Name.shortName>::value == -1),
        "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>::value == -1,
                  "Duplicated name");
    FlagArgInitializer<Name, ID>::init(std::forward<T>(args)...);
    return Parser<ID, tuple_append_t<Args, FlagArg<Name, ID>>, PArgs, HArg,
                  SubParsers>(std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  auto addHelp() {
    static_assert((SearchIndexFromShortName<Args, Name.shortName>::value == -1),
                  "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>::value == -1,
                  "Duplicated name");
    return Parser<ID, Args, PArgs, HelpArg<Name, ID>, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  auto addHelp(string_view help) {
    static_assert((SearchIndexFromShortName<Args, Name.shortName>::value == -1),
                  "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>::value == -1,
                  "Duplicated name");
    static_assert(!Name.containsInvalidChar(), "Name has invalid char");
    static_assert(Name.hasValidNameLength(),
                  "Short name can't be more than one charactor");
    this->info_->help = help;
    return Parser<ID, Args, PArgs, HelpArg<Name, ID>, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name>
  auto getArg() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      if constexpr (SearchIndex<PArgs, Name>::value != -1) {
        return tuple_element_t<SearchIndex<PArgs, Name>::value, PArgs>::value;
      } else {
        static_assert(SearchIndex<Args, Name>::value != -1,
                      "Argument does not exist");
        return remove_cvref_t<decltype(get<SearchIndex<Args, Name>::value>(
            declval<Args>()))>::value;
      }
    } else {
      static_assert(SearchIndex<Args, Name>::value != -1,
                    "Argument does not exist");
      return remove_cvref_t<decltype(get<SearchIndex<Args, Name>::value>(
          declval<Args>()))>::value;
    }
  }

  template <ArgName Name>
  auto& getParser() {
    if constexpr (is_same_v<SubParsers, tuple<>>) {
      static_assert(false, "Parser has no sub parser");
    }
    static_assert(!(SearchIndex<SubParsers, Name>::value == -1),
                  "Could not find subparser");
    return get<SearchIndex<SubParsers, Name>::value>(subParsers).parser.get();
  }

  template <ArgName Name>
  auto isAssigned() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      if constexpr (string_view(Name) == string_view(PArgs::name)) {
        return PArgs::assigned;
      } else {
        return remove_cvref_t<decltype(get<SearchIndex<Args, Name>::value>(
            declval<Args>()))>::assigned;
      }
    } else {
      return remove_cvref_t<decltype(get<SearchIndex<Args, Name>::value>(
          declval<Args>()))>::assigned;
    }
  }

  /*!
   * Add subcommand
   */
  template <ArgName Name, class T>
  auto addParser(T& sub_parser, Description description = {""}) {
    auto s = make_tuple(
        SubParser<Name, T>{ref(sub_parser), description.description});
    auto sub_parsers = tuple_cat(subParsers, s);
    return Parser<ID, Args, PArgs, HArg, decltype(sub_parsers)>(
        std::move(this->info_), sub_parsers);
  }

  auto resetArgs() -> void;

  auto addUsageHelp(string_view usage) {
    this->info_->usage = usage;
  }

  auto addSubcommandHelp(string_view subcommand_help) {
    this->info_->subcommand_help = subcommand_help;
  }

  auto addPositionalArgumentHelp(string_view positional_argument_help) {
    this->info_->positional_argument_help = positional_argument_help;
  }

  auto addOptionsHelp(string_view options_help) {
    this->info_->options_help = options_help;
  }

 private:
  auto setArg(string_view key, span<string_view> val) const -> void;
  auto setArg(span<char> key, span<string_view> val) const -> void;

 public:
  auto parse(int argc, char* argv[]) -> void;
  [[nodiscard]] string formatHelp(bool no_color = false) const;

  explicit operator bool() const {
    return this->parsed_;
  }
};

}  // namespace Argo


namespace Argo {

using namespace std;

inline auto splitStringView(string_view str, char delimeter)
    -> vector<string_view> {
  vector<string_view> ret;
  while (str.contains(delimeter)) {
    auto pos = str.find(delimeter);
    ret.push_back(str.substr(0, pos));
    str = str.substr(pos + 1);
  }
  ret.push_back(str);
  return ret;
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::resetArgs() -> void {
  ValueReset<Args>();
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::setArg(
    string_view key, span<string_view> val) const -> void {
  if constexpr (!is_same_v<HArg, void>) {
    if (key == HArg::name) {
      println("{}", formatHelp());
      exit(0);
    }
  }
  Assigner<Args, PArgs>::assign(key, val);
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::setArg(
    span<char> key, span<string_view> val) const -> void {
  if constexpr (!is_same_v<HArg, void>) {
    for (const auto& i : key) {
      if constexpr (HArg::name.shortName != '\0') {
        if (i == HArg::name.shortName) {
          println("{}", formatHelp());
          exit(0);
        }
      }
    }
  }
  Assigner<Args, PArgs>::assign(key, val);
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::parse(int argc, char* argv[])
    -> void {
  if (this->parsed_) [[unlikely]] {
    throw ParseError("Cannot parse twice");
  }
  auto assigned_keys = AssignChecker<Args>::check();
  if (!assigned_keys.empty()) [[unlikely]] {
    throw ParseError(format("keys {} already assigned", assigned_keys));
  }

  int64_t subcmd_found_idx = -1;
  int64_t cmd_end_pos = argc;
  if constexpr (!is_same_v<SubParsers, tuple<>>) {
    for (int i = argc - 1; i > 0; i--) {
      subcmd_found_idx = ParserIndex(subParsers, argv[i]);
      if (subcmd_found_idx != -1) {
        cmd_end_pos = i;
        break;
      }
    }
  }

  string_view key{};
  vector<char> short_keys{};
  short_keys.reserve(10);
  vector<string_view> values{};
  values.reserve(10);

  assert(this->info_);  // this->info_ cannot be nullptr
  if (!this->info_->program_name) {
    this->info_->program_name = string_view(argv[0]);
  }

  for (int i = 1; i < cmd_end_pos; i++) {
    string_view arg = argv[i];
    if (arg.starts_with('-')) {
      if (arg.starts_with("--")) {  // start with --
        if (!key.empty()) {
          this->setArg(key, values);
          key = "";
          values.clear();
        } else if (!short_keys.empty()) {
          this->setArg(short_keys, values);
          short_keys.clear();
          values.clear();
        } else if (!values.empty()) {
          if constexpr (!is_same_v<PArgs, tuple<>>) {
            this->setArg(key, values);
            values.clear();
          }
        }
        if (arg.contains('=')) {
          auto equal_pos = arg.find('=');
          auto value = vector<string_view>{arg.substr(equal_pos + 1)};
          this->setArg(arg.substr(2, equal_pos - 2), value);
          continue;
        }
        key = arg.substr(2);
        if (i == (cmd_end_pos - 1)) {
          this->setArg(key, {});
        }
        continue;
      }
      if (!key.empty()) {
        this->setArg(key, values);
        key = "";
        values.clear();
      } else if (!short_keys.empty()) {
        this->setArg(short_keys, values);
        short_keys.clear();
        values.clear();
      } else if (!values.empty()) {
        if constexpr (!is_same_v<PArgs, tuple<>>) {
          this->setArg(key, values);
          values.clear();
        }
      }
      if (key.empty() && short_keys.empty()) {
        for (const auto& j : arg.substr(1)) {
          short_keys.push_back(j);
        }
        if (i == (cmd_end_pos - 1)) {
          this->setArg(short_keys, {});
        }
        continue;
      }
    } else {
      if constexpr (is_same_v<PArgs, tuple<>>) {
        if (key.empty() && short_keys.empty()) {
          throw InvalidArgument(format("No keys specified"));
        }
      }
      values.push_back(arg);

      if (i == cmd_end_pos - 1) {
        if (!key.empty()) {
          this->setArg(key, values);
        } else if (!short_keys.empty()) {
          this->setArg(short_keys, values);
        } else {
          if constexpr (is_same_v<PArgs, tuple<>>) {
            throw InvalidArgument(format("No keys specified"));
          } else {
            this->setArg(key, values);
          }
        }
      }
    }
  }

  // Check Required values
  auto required_keys = RequiredChecker<  //
      decltype(tuple_cat(declval<Args>(), declval<PArgs>()))>::check();
  if (!required_keys.empty()) {
    throw InvalidArgument(format("Requried {}", required_keys));
  }
  if (subcmd_found_idx != -1) {
    MetaParse(subParsers, subcmd_found_idx, argc - cmd_end_pos,
              &argv[cmd_end_pos]);
  }
  this->parsed_ = true;
}

struct AnsiEscapeCode {
  bool isEnabled;

  string bold = "\x1B[1m";
  string underline = "\x1B[4m";
  string reset = "\x1B[0m";

  [[nodiscard]] auto getBold() const {
    return isEnabled ? bold : "";
  }

  [[nodiscard]] auto getUnderline() const {
    return isEnabled ? underline : "";
  }

  [[nodiscard]] auto getReset() const {
    return isEnabled ? reset : "";
  }

  [[nodiscard]] auto getBoldUnderline() const {
    return isEnabled ? bold + underline : "";
  }
};

inline auto createUsageSection(const auto& program_name,
                               [[maybe_unused]] const auto& help_info,
                               const auto& sub_commands) {
  string ret;
  ret.append(program_name);

  // for (const auto& i : help_info) {
  //   ret.push_back(' ');
  //   if (!i.required) {
  //     ret.push_back('[');
  //   }
  //   ret.append("--");
  //   ret.append(i.name);
  //   if (!i.typeName.empty()) {
  //     ret.push_back(' ');
  //     ret.append(i.typeName);
  //   }
  //   if (!i.required) {
  //     ret.push_back(']');
  //   }
  // }

  ret.append(" [options...]");

  if (!sub_commands.empty()) {
    ret.append(" {");
    for (const auto& command : sub_commands) {
      ret.append(command.name);
      ret.push_back(',');
    }
    ret.pop_back();
    ret.push_back('}');
  }
  ret.push_back('\n');
  return ret;
}

inline auto createSubcommandSection(const auto& ansi,
                                    const auto& sub_commands) {
  string ret;
  size_t max_command_length = 0;
  for (const auto& command : sub_commands) {
    if (max_command_length < command.name.size()) {
      max_command_length = command.name.size();
    }
  }
  for (const auto& command : sub_commands) {
    ret.push_back('\n');
    auto description = splitStringView(command.description, '\n');

    ret.append(format("  {}{} {}{} {}", ansi.getBold(), command.name,
                      string(max_command_length - command.name.size(), ' '),
                      ansi.getReset(), description[0]));
    for (size_t i = 1; i < description.size(); i++) {
      ret.push_back('\n');
      ret.append(format("    {}{}",  //
                        string(max_command_length, ' '), description[i]));
    }

    // erase trailing spaces
    auto pos = ret.find_last_not_of(' ');
    ret = ret.substr(0, pos + 1);
  }

  ret.push_back('\n');
  return ret;
}

inline auto createOptionsSection(const auto& ansi, const auto& help_info) {
  string ret;
  size_t max_name_len = 0;
  for (const auto& option : help_info) {
    if (max_name_len < option.name.size() + option.typeName.size()) {
      max_name_len = option.name.size() + option.typeName.size();
    }
  }
  for (const auto& option : help_info) {
    ret.push_back('\n');
    auto description = splitStringView(option.description, '\n');

    ret.append(format(
        "  {}{} --{} {}{}{}  {}",
        (option.shortName == '\0') ? "   " : format("-{},", option.shortName),
        ansi.getBold(), option.name, ansi.getReset(), option.typeName,
        string(max_name_len - option.name.size() - option.typeName.size(), ' '),
        description[0]));
    for (size_t i = 1; i < description.size(); i++) {
      ret.push_back('\n');
      ret.append(
          format("      {}     {}", string(max_name_len, ' '), description[i]));
    }

    auto pos = ret.find_last_not_of(' ');
    ret = ret.substr(0, pos + 1);
  }
  return ret;
}

template <class PArgs>
auto createPositionalArgumentSection(const auto& ansi) {
  string ret;

  vector<ArgInfo> info = HelpGenerator<PArgs>::generate();

  for (const auto& i : info) {
    ret.push_back('\n');
    auto desc = splitStringView(i.description, '\n');
    ret.append(format("  {}{}{}  {}", ansi.getBold(), string_view(i.name),
                      ansi.getReset(), desc[0]));
    for (size_t i = 1; i < desc.size(); i++) {
      ret.push_back('\n');
      ret.append(format("{}{}", string(8, ' '), desc[i]));
    }
  }

  ret.push_back('\n');
  return ret;
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArgs, HArg, SubParsers>::formatHelp(bool no_color) const
    -> string {
  string ret;

  AnsiEscapeCode ansi((::isatty(1) != 0) and !no_color);

  assert(this->info_);  // this->info_ cannot be nullptr

  vector<ArgInfo> help_info;
  if constexpr (is_same_v<HArg, void>) {
    help_info = HelpGenerator<Args>::generate();
  } else {
    help_info = HelpGenerator<tuple_append_t<Args, HArg>>::generate();
  }

  auto sub_commands = SubParserInfo(subParsers);

  if (this->info_->help) {
    ret.append(this->info_->help.value());
    ret.push_back('\n');
  } else {
    // Description Section
    if (this->info_->description) {
      ret.append(this->info_->description.value());
      ret.push_back('\n');
    }

    // Usage Section
    ret.push_back('\n');
    ret.append(ansi.getBoldUnderline() + "USAGE:" + ansi.getReset() + "\n");
    ret.append("  ");

    ret.append(this->info_->usage.value_or(
        createUsageSection(this->info_->program_name.value_or("no_name"),
                           help_info, sub_commands)));

    // Subcommand Section
    if constexpr (!is_same_v<SubParsers, tuple<>>) {
      ret.push_back('\n');
      ret.append(ansi.getBoldUnderline() + "Subcommands:" + ansi.getReset());
      ret.append(this->info_->subcommand_help.value_or(
          createSubcommandSection(ansi, sub_commands)));
    }

    if constexpr (!is_same_v<PArgs, tuple<>>) {
      ret.push_back('\n');
      ret.append(ansi.getBoldUnderline() +
                 "Positional Argument:" + ansi.getReset());
      ret.append(this->info_->positional_argument_help.value_or(
          createPositionalArgumentSection<PArgs>(ansi)));
    }

    // Options section
    if (!help_info.empty()) {
      ret.push_back('\n');
      ret.append(ansi.getBoldUnderline() + "Options:" + ansi.getReset());
      ret.append(this->info_->options_help.value_or(
          createOptionsSection(ansi, help_info)));
    }
  }

  return ret;
}

}  // namespace Argo

