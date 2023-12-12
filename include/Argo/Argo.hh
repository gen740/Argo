#pragma once

#include <unistd.h>

#include <array>
#include <cassert>
#include <charconv>
#include <concepts>
#include <cstring>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#define ARGO_ALWAYS_INLINE __attribute__((always_inline))


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
using std::type_identity;

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

template <class... T>
struct type_sequence {};

template <class T>
struct make_type_sequence {};

template <class... T>
struct make_type_sequence<tuple<T...>> {
  using type = type_sequence<T...>;
};

template <class T>
using make_type_sequence_t = make_type_sequence<T>::type;

template <class Tuple, class T>
ARGO_ALWAYS_INLINE constexpr auto tuple_type_visit(T fun) {
  [&fun]<class... U>(type_sequence<U...>) ARGO_ALWAYS_INLINE {
    (fun(type_identity<U>()), ...);
  }(make_type_sequence_t<Tuple>());
}

template <class Tuple, class T>
ARGO_ALWAYS_INLINE constexpr auto tuple_type_or_visit(T fun) -> bool {
  return [&fun]<class... U>(type_sequence<U...>) ARGO_ALWAYS_INLINE {
    return (fun(type_identity<U>()) || ...);
  }(make_type_sequence_t<Tuple>());
}

template <class Tuple, class T>
ARGO_ALWAYS_INLINE constexpr auto tuple_type_and_visit(T fun) -> bool {
  return [&fun]<class... U>(type_sequence<U...>) ARGO_ALWAYS_INLINE {
    return (fun(type_identity<U>()) && ...);
  }(make_type_sequence_t<Tuple>());
}

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
  [[noreturn]] auto isValid(const T& /* unused */,
                            span<string_view> /* unuesd */) const -> bool {
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

using namespace std;

/*!
 * ArgName which holds argument name
 */
template <size_t N>
struct ArgName {
  char name[N] = {};
  char shortName = '\0';
  size_t nameLen = N;

  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval ArgName(const char (&lhs)[N + 1]) {
    for (size_t i = 0; i < N; i++) {
      if (lhs[i] == ',') {
        nameLen = i;
        shortName = lhs[i + 1];
        return;
      }
      this->name[i] = lhs[i];
    }
  };

  [[nodiscard]] constexpr char operator[](size_t idx) const {
    return this->name[idx];
  }

  constexpr char& operator[](size_t idx) {
    return this->name[idx];
  }

  [[nodiscard]] constexpr auto begin() const {
    return &this->name[0];
  }

  [[nodiscard]] constexpr auto end() const {
    return &this->name[this->nameLen];
  }

  [[nodiscard]] constexpr auto size() const {
    return N;
  }

  [[nodiscard]] friend constexpr auto begin(const ArgName& lhs) {
    return lhs.begin();
  }

  [[nodiscard]] friend constexpr auto end(const ArgName& lhs) {
    return lhs.end();
  }

  template <size_t M>
  [[nodiscard]] ARGO_ALWAYS_INLINE constexpr auto operator==(
      const ArgName<M>& lhs) -> bool {
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
  [[nodiscard]] ARGO_ALWAYS_INLINE constexpr auto operator==(
      const ArgName<M>& lhs) const -> bool {
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
  [[nodiscard]] ARGO_ALWAYS_INLINE constexpr operator string_view() const {
    return string_view(this->begin(), this->end());
  }

  [[nodiscard]] ARGO_ALWAYS_INLINE consteval auto hasValidNameLength() const
      -> bool {
    if (this->shortName == '\0') {
      return true;
    }
    return (N - this->nameLen) == 2;
  }
};

template <size_t N>
ArgName(const char (&)[N]) -> ArgName<N - 1>;

template <class T>
concept ArgNameType = requires(T& x) {
  static_cast<string_view>(x);
  is_same_v<decltype(x.shortName), char>;
};

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
  consteval ParserID(int id) : id(id){};

  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval ParserID(const char (&id)[N + 1]) {
    for (size_t i = 0; i < N; i++) {
      this->id.idName[i] = id[i];
    }
  };
};

ParserID(int) -> ParserID<1>;

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

template <size_t N>
struct String {
  char str_[N] = {};

  String() = default;

  consteval explicit String(const char (&str)[N + 1]) {
    for (size_t i = 0; i < N; i++) {
      str_[i] = str[i];
    }
  };

  consteval explicit operator string() const {
    return string(str_, N);
  }

  consteval explicit operator string_view() const {
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
  using baseType = conditional_t<                              //
      is_array_v<Type>,                                        //
      array_base_t<Type>,                                      //
      conditional_t<                                           //
          is_vector_v<Type>,                                   //
          vector_base_t<Type>,                                 //
          Type                                                 //
          >                                                    //
      >;
  static constexpr auto name = Name;
  static constexpr auto id = ID;
  inline static string_view description;
  inline static bool assigned = false;
  inline static bool required = Required;

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = TNArgs;
  inline static function<type(string_view)> caster = nullptr;
  inline static function<void(const type& value, span<string_view>,
                              string_view)>
      validator = nullptr;
  inline static function<void(type&, span<string_view>)> callback = nullptr;
  inline static constexpr auto typeName = get_type_name<type, TNArgs>();
};

struct FlagArgTag {};

template <ArgName Name, ParserID ID>
struct FlagArg : FlagArgTag {
  using type = bool;
  using baseType = bool;

  static constexpr auto name = Name;
  static constexpr auto id = ID;
  inline static bool assigned = false;
  inline static string_view description;
  inline static bool required = false;

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = NArgs(-1);
  inline static function<void()> callback = nullptr;
  inline static constexpr auto typeName = String("");
};

struct HelpArgTag {};

template <ArgName Name, ParserID ID>
struct HelpArg : HelpArgTag, FlagArgTag {
  using type = bool;
  using baseType = bool;
  static constexpr auto name = Name;
  static constexpr auto id = ID;

  inline static bool assigned = false;
  inline static string_view description = "Print help information";
  inline static bool required = false;

  inline static type value = {};
  inline static type defaultValue = {};

  inline static constexpr NArgs nargs = NArgs(-1);
  inline static function<void()> callback = nullptr;
  inline static constexpr auto typeName = String("");
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

inline constexpr auto description(string_view desc) -> Description {
  return {.description = desc};
}

template <class T>
inline constexpr auto explicitDefault(T value) -> ExlicitDefaultValue<T> {
  return {.explicit_default_value = value};
}

template <class T>
inline constexpr auto implicitDefault(T value) -> ImplicitDefaultValue<T> {
  return {.implicit_default_value = value};
}

template <class Type, ArgName Name, NArgs nargs, bool Required, ParserID ID,
          class... Args>
inline constexpr auto ArgInitializer(Args... args) {
  (
      [&args]() {
        using Arg = Arg<Type, Name, nargs, Required, ID>;
        if constexpr (is_same_v<Args, Description>) {
          Arg::description = args.description;
        } else if constexpr (derived_from<remove_cvref_t<Args>,
                                          Validation::ValidationBase>) {
          static_assert(is_invocable_v<Args, typename Arg::type,
                                       span<string_view>, string_view>,
                        "Invalid validator");
          Arg::validator = args;
        } else if constexpr (derived_from<remove_cvref_t<Args>,
                                          ImplicitDefaultValueTag>) {
          Arg::defaultValue = static_cast<Type>(args.implicit_default_value);
        } else if constexpr (derived_from<remove_cvref_t<Args>,
                                          ExplicitDefaultValueTag>) {
          Arg::value = static_cast<Type>(args.explicit_default_value);
        } else if constexpr (is_invocable_v<Args, typename Arg::type&,
                                            span<string_view>>) {
          Arg::callback = args;
        } else {
          static_assert(false, "Invalid argument");
        }
      }(),
      ...);
}

template <ArgName Name, ParserID ID, class... Args>
inline constexpr auto FlagArgInitializer(Args... args) {
  (
      [&args]() {
        using FlagArg = FlagArg<Name, ID>;
        if constexpr (is_same_v<Args, Description>) {
          FlagArg::description = args.description;
        } else if constexpr (derived_from<remove_cvref_t<Args>,
                                          Validation::ValidationBase>) {
          static_assert(false, "Flag cannot have validator");
        } else if constexpr (derived_from<remove_cvref_t<Args>,
                                          ImplicitDefaultValueTag>) {
          static_assert(false, "Flag cannot have implicit default value");
        } else if constexpr (derived_from<remove_cvref_t<Args>,
                                          ExplicitDefaultValueTag>) {
          static_assert(false, "Flag cannot have explicit default value");
        } else if constexpr (is_invocable_v<Args>) {
          FlagArg::callback = args;
        } else {
          static_assert(false, "Invalid argument");
        }
      }(),
      ...);
}

};  // namespace Argo


namespace Argo {

using namespace std;

struct ArgInfo {
  string_view name;
  char shortName;
  string_view description;
  bool required;
  string_view typeName;
};

template <class Args>
struct HelpGenerator {};

template <class... Args>
struct HelpGenerator<tuple<Args...>> {
  ARGO_ALWAYS_INLINE constexpr static auto generate() -> vector<ArgInfo> {
    vector<ArgInfo> ret;
    (
        [&ret]<class T>() ARGO_ALWAYS_INLINE {
          ret.emplace_back(
              string_view(Args::name).substr(0, Args::name.nameLen),
              Args::name.shortName, Args::description, Args::required,
              string_view(Args::typeName));
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
ARGO_ALWAYS_INLINE constexpr auto SubParserInfo(T subparsers) {
  vector<SubCommandInfo> ret{};
  if constexpr (!is_same_v<T, tuple<>>) {
    apply(
        [&ret]<class... Parser>(Parser... parser) ARGO_ALWAYS_INLINE {
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
ARGO_ALWAYS_INLINE constexpr inline auto GetNameFromShortName(char key) {
  auto name = string_view();
  if ([&name, &key]<class... T>(type_sequence<T...>) ARGO_ALWAYS_INLINE {
        return ([&name, &key] {
          if (T::name.shortName == key) {
            name = string_view(T::name);
            return true;
          }
          return false;
        }() || ...);
      }(make_type_sequence_t<Arguments>())) {
    return name;
  }
  throw ParserInternalError("Fail to lookup");
}

/*!
 * Index Search meta function
 */
template <class Tuple, ArgName Name>
consteval auto SearchIndex() {
  int value = -1;
  if (![&value]<class... T>(type_sequence<T...>) {
        return ((value++, Name == T::name) || ...);
      }(make_type_sequence_t<Tuple>())) {
    return -1;
  }
  return value;
}

/*!
 * Index Search meta function
 */
template <class Tuple, char C>
consteval auto SearchIndexFromShortName() {
  int value = -1;
  if (![&value]<class... T>(type_sequence<T...>) {
        return ((value++, C == T::name.shortName) || ...);
      }(make_type_sequence_t<Tuple>())) {
    return -1;
  }
  return value;
}

};  // namespace Argo


namespace Argo {

using namespace std;

/*!
 * Helper class of assigning value
 */
template <class Type>
ARGO_ALWAYS_INLINE constexpr auto ArgCaster(const string_view& value) -> Type {
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
    throw InvalidArgument("Invalid argument expect bool");
  } else if constexpr (is_integral_v<Type>) {
    Type ret;
    from_chars(value.begin(), value.end(), ret);
    return ret;
  } else if constexpr (is_floating_point_v<Type>) {
    return static_cast<Type>(stod(string(value)));
  } else if constexpr (is_same_v<Type, const char*>) {
    return value.data();
  } else {
    return static_cast<Type>(value);
  }
}

template <class... T, size_t... N>
ARGO_ALWAYS_INLINE constexpr auto TupleAssign(tuple<T...>& t,
                                              span<string_view> v,
                                              index_sequence<N...> /* unused */)
    -> void {
  ((get<N>(t) = ArgCaster<remove_cvref_t<decltype(get<N>(t))>>(v[N])), ...);
}

template <ArgType Arg>
ARGO_ALWAYS_INLINE constexpr auto AfterAssign(const span<string_view>& values)
    -> void {
  Arg::assigned = true;
  if (Arg::validator) {
    Arg::validator(Arg::value, values, string_view(Arg::name));
  }
  if (Arg::callback) {
    Arg::callback(Arg::value, values);
  }
}

template <ArgType Arg>
ARGO_ALWAYS_INLINE constexpr auto ValiadicArgAssign(
    const span<string_view>& values) -> void {
  Arg::value.resize(values.size());
  for (size_t i = 0; i < values.size(); i++) {
    Arg::value[i] = ArgCaster<typename Arg::baseType>(values[i]);
  }
  AfterAssign<Arg>(values);
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto NLengthArgAssign(span<string_view>& values)
    -> void {
  if (Arg::nargs.getNargs() > values.size()) {
    throw Argo::InvalidArgument(format("Argument {}: invalid argument {}",
                                       string_view(Arg::name), values));
  }
  if constexpr (is_array_v<typename Arg::type>) {
    for (size_t i = 0; i < Arg::nargs.getNargs(); i++) {
      Arg::value[i] = ArgCaster<typename Arg::baseType>(values[i]);
    }
  } else if constexpr (is_vector_v<typename Arg::type>) {
    Arg::value.resize(Arg::nargs.getNargs());
    for (size_t i = 0; i < Arg::nargs.getNargs(); i++) {
      Arg::value[i] = ArgCaster<typename Arg::baseType>(values[i]);
    }
  } else if constexpr (is_tuple_v<typename Arg::type>) {
    TupleAssign(Arg::value, values,
                make_index_sequence<tuple_size_v<typename Arg::type>>());
  } else {
    static_assert(false, "Invalid Type");
  }
  AfterAssign<Arg>(values.subspan(0, Arg::nargs.getNargs()));
  values = values.subspan(Arg::nargs.getNargs());
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto ZeroOrOneArgAssign(span<string_view>& values)
    -> void {
  if (values.empty()) {
    Arg::value = Arg::defaultValue;
  } else {
    Arg::value = ArgCaster<typename Arg::type>(values[0]);
  }
  AfterAssign<Arg>(values.subspan(0, 1));
  values = values.subspan(1);
}

template <class PArgs>
ARGO_ALWAYS_INLINE constexpr auto PArgAssigner(span<string_view> values)
    -> bool {
  return [&values]<class... Arg>(type_sequence<Arg...>) ARGO_ALWAYS_INLINE {
    return ([&values] ARGO_ALWAYS_INLINE {
      if (Arg::assigned) {
        return false;
      }
      if constexpr (Arg::nargs.getNargsChar() == '+') {
        ValiadicArgAssign<Arg>(values);
        return true;
      }
      if constexpr (Arg::nargs.getNargs() > 0) {
        NLengthArgAssign<Arg>(values);
        return values.empty();
      }
    }() || ...);
  }(make_type_sequence_t<PArgs>());
}

template <>
ARGO_ALWAYS_INLINE constexpr auto PArgAssigner<std::tuple<>>(
    span<string_view> /*unused*/) -> bool {
  return true;
}

template <class Head, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto AssignOneArg(const string_view& key,
                                               span<string_view> values)
    -> bool {
  if constexpr (derived_from<Head, FlagArgTag>) {
    if (!values.empty()) {
      if constexpr (is_same_v<PArgs, tuple<>>) {
        throw Argo::InvalidArgument(format("Flag {} can not take value", key));
      } else {
        PArgAssigner<PArgs>(values);
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
      ZeroOrOneArgAssign<Head>(values);
      if (values.empty()) {
        return true;
      }
      return PArgAssigner<PArgs>(values);
    } else if constexpr (Head::nargs.getNargsChar() == '*') {
      if (values.empty()) {
        Head::value = Head::defaultValue;
        Head::assigned = true;
        return true;
      }
      ValiadicArgAssign<Head>(values);
      return true;
    } else if constexpr (Head::nargs.getNargsChar() == '+') {
      if (values.empty()) {
        throw Argo::InvalidArgument(
            format("Argument {} should take more than one value", key));
      }
      ValiadicArgAssign<Head>(values);
      return true;
    } else if constexpr (Head::nargs.getNargs() == 1) {
      if (values.empty()) {
        throw Argo::InvalidArgument(
            format("Argument {} should take exactly one value but zero", key));
      }
      ZeroOrOneArgAssign<Head>(values);
      if (values.empty()) {
        return true;
      }
      return PArgAssigner<PArgs>(values);
    } else {
      NLengthArgAssign<Head>(values);
      if (values.empty()) {
        return true;
      }
      return PArgAssigner<PArgs>(values);
    }
  }
  return false;
}

template <class Args, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto assignArg(const string_view& key,
                                            const span<string_view>& values) {
  [&key, &values]<size_t... Is>(index_sequence<Is...>)
      ARGO_ALWAYS_INLINE -> void {
        if (!(... ||
              (string_view(tuple_element_t<Is, Args>::name) == key and
               AssignOneArg<tuple_element_t<Is, Args>, PArgs>(key, values)))) {
          throw Argo::InvalidArgument(format("Invalid argument {}", key));
        }
      }(make_index_sequence<tuple_size_v<Args>>());
}

template <class Arguments, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto Assigner(string_view key,
                                           span<string_view> values) -> void {
  if (key.empty()) {
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      if (!PArgAssigner<PArgs>(values)) {
        throw InvalidArgument(format("Duplicated positional argument"));
      }
      return;
    } else {
      throw Argo::InvalidArgument(format("Assigner: Invalid argument {}", key));
    }
  }
  assignArg<Arguments, PArgs>(key, values);
}

template <class Arguments, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto Assigner(span<char> key,
                                           span<string_view> values) -> void {
  for (size_t i = 0; i < key.size(); i++) {
    auto found_key = GetNameFromShortName<Arguments>(key[i]);
    if (i == key.size() - 1) {
      assignArg<Arguments, PArgs>(found_key, values);
    } else {
      assignArg<Arguments, PArgs>(found_key, {});
    }
  }
}

template <class Args>
ARGO_ALWAYS_INLINE constexpr auto ValueReset() -> void {
  []<size_t... Is>(index_sequence<Is...>) ARGO_ALWAYS_INLINE {
    (..., []<ArgType T>() ARGO_ALWAYS_INLINE {
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

template <ArgName Name, class Parser>
struct SubParser {
  static constexpr auto name = Name;
  reference_wrapper<Parser> parser;
  string_view description;
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
ARGO_ALWAYS_INLINE constexpr auto MetaParse(SubParsers sub_parsers, int index,
                                            int argc, char** argv) -> bool {
  return apply(
      [&](auto&&... s) ARGO_ALWAYS_INLINE {
        int64_t idx = -1;
        return (... || (idx++, idx == index &&
                                   (s.parser.get().parse(argc, argv), true)));
      },
      sub_parsers);
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
ARGO_ALWAYS_INLINE constexpr auto ParserIndex(SubParsers sub_parsers,  //
                                              string_view key) -> int64_t {
  return apply(
      [&](auto&&... s) ARGO_ALWAYS_INLINE {
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
  constexpr auto createArg(T... args) {
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
      static_assert(SearchIndex<PArgs, Name>() == -1, "Duplicated name");
    }
    static_assert((Name.shortName == '\0') ||
                      (SearchIndexFromShortName<Args, Name.shortName>() == -1),
                  "Duplicated short name");
    static_assert(                        //
        SearchIndex<Args, Name>() == -1,  //
        "Duplicated name");
    static_assert(                         //
        (nargs.getNargs() > 0              //
         || nargs.getNargsChar() == '?'    //
         || nargs.getNargsChar() == '+'    //
         || nargs.getNargsChar() == '*'),  //
        "nargs must be '?', '+', '*' or int");

    ArgInitializer<Type, Name, nargs, required, ID>(std::forward<T>(args)...);
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
  constexpr auto addArg(T... args) {
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
  constexpr auto addPositionalArg(T... args) {
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
  constexpr auto addFlag(T... args) {
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      static_assert(SearchIndex<PArgs, Name>() == -1, "Duplicated name");
    }
    static_assert((Name.shortName == '\0') ||
                      (SearchIndexFromShortName<Args, Name.shortName>() == -1),
                  "Duplicated short name");
    static_assert(SearchIndex<Args, Name>() == -1, "Duplicated name");
    FlagArgInitializer<Name, ID>(std::forward<T>(args)...);
    return Parser<ID, tuple_append_t<Args, FlagArg<Name, ID>>, PArgs, HArg,
                  SubParsers>(std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  constexpr auto addHelp() {
    static_assert((SearchIndexFromShortName<Args, Name.shortName>() == -1),
                  "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>() == -1, "Duplicated name");
    return Parser<ID, Args, PArgs, HelpArg<Name, ID>, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  constexpr auto addHelp(string_view help) {
    static_assert((SearchIndexFromShortName<Args, Name.shortName>() == -1),
                  "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>() == -1, "Duplicated name");
    static_assert(Name.hasValidNameLength(),
                  "Short name can't be more than one charactor");
    this->info_->help = help;
    return Parser<ID, Args, PArgs, HelpArg<Name, ID>, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name>
  constexpr auto getArg() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      if constexpr (SearchIndex<PArgs, Name>() != -1) {
        return tuple_element_t<SearchIndex<PArgs, Name>(), PArgs>::value;
      } else {
        static_assert(SearchIndex<Args, Name>() != -1,
                      "Argument does not exist");
        return remove_cvref_t<decltype(get<SearchIndex<Args, Name>()>(
            declval<Args>()))>::value;
      }
    } else {
      static_assert(SearchIndex<Args, Name>() != -1, "Argument does not exist");
      return remove_cvref_t<decltype(get<SearchIndex<Args, Name>()>(
          declval<Args>()))>::value;
    }
  }

  template <ArgName Name>
  constexpr auto& getParser() {
    if constexpr (is_same_v<SubParsers, tuple<>>) {
      static_assert(false, "Parser has no sub parser");
    }
    static_assert(!(SearchIndex<SubParsers, Name>() == -1),
                  "Could not find subparser");
    return get<SearchIndex<SubParsers, Name>()>(subParsers).parser.get();
  }

  template <ArgName Name>
  constexpr auto isAssigned() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!is_same_v<PArgs, tuple<>>) {
      if constexpr (string_view(Name) == string_view(PArgs::name)) {
        return PArgs::assigned;
      } else {
        return remove_cvref_t<decltype(get<SearchIndex<Args, Name>()>(
            declval<Args>()))>::assigned;
      }
    } else {
      return remove_cvref_t<decltype(get<SearchIndex<Args, Name>()>(
          declval<Args>()))>::assigned;
    }
  }

  /*!
   * Add subcommand
   */
  template <ArgName Name, class T>
  ARGO_ALWAYS_INLINE constexpr auto addParser(T& sub_parser,
                                              Description description = {""}) {
    auto s = make_tuple(
        SubParser<Name, T>{ref(sub_parser), description.description});
    auto sub_parsers = tuple_cat(subParsers, s);
    return Parser<ID, Args, PArgs, HArg, decltype(sub_parsers)>(
        std::move(this->info_), sub_parsers);
  }

  ARGO_ALWAYS_INLINE constexpr auto resetArgs() -> void;

  ARGO_ALWAYS_INLINE constexpr auto addUsageHelp(string_view usage) {
    this->info_->usage = usage;
  }

  ARGO_ALWAYS_INLINE constexpr auto addSubcommandHelp(
      string_view subcommand_help) {
    this->info_->subcommand_help = subcommand_help;
  }

  ARGO_ALWAYS_INLINE constexpr auto addPositionalArgumentHelp(
      string_view positional_argument_help) {
    this->info_->positional_argument_help = positional_argument_help;
  }

  ARGO_ALWAYS_INLINE constexpr auto addOptionsHelp(string_view options_help) {
    this->info_->options_help = options_help;
  }

 private:
  ARGO_ALWAYS_INLINE constexpr auto setArg(string_view key,
                                           span<string_view> val) const -> void;
  ARGO_ALWAYS_INLINE constexpr auto setArg(span<char> key,
                                           span<string_view> val) const -> void;

 public:
  ARGO_ALWAYS_INLINE constexpr auto parse(int argc, char* argv[]) -> void;
  [[nodiscard]] constexpr string formatHelp(bool no_color = false) const;

  explicit constexpr operator bool() const {
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
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::resetArgs() -> void {
  ValueReset<Args>();
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::setArg(
    string_view key, span<string_view> val) const -> void {
  if constexpr (!is_same_v<HArg, void>) {
    if (key == HArg::name) {
      std::cout << formatHelp() << '\n';
      exit(0);
    }
  }
  Assigner<Args, PArgs>(key, val);
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::setArg(
    span<char> key, span<string_view> val) const -> void {
  if constexpr (!is_same_v<HArg, void>) {
    for (const auto& i : key) {
      if constexpr (HArg::name.shortName != '\0') {
        if (i == HArg::name.shortName) {
          std::cout << formatHelp() << '\n';
          exit(0);
        }
      }
    }
  }
  Assigner<Args, PArgs>(key, val);
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
inline constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::parse(
    int argc, char* argv[]) -> void {
  if (this->parsed_) [[unlikely]] {
    throw ParseError("Cannot parse twice");
  }
  auto assigned_keys = vector<string_view>();
  tuple_type_visit<decltype(tuple_cat(declval<Args>(), declval<PArgs>()))>(
      [&assigned_keys]<class T>(T) {
        if (T::type::assigned) {
          assigned_keys.push_back(string_view(T::type::name));
        }
      });
  if (!assigned_keys.empty()) [[unlikely]] {
    throw ParseError(format("keys {} already assigned", assigned_keys));
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

  // Search for subcommand
  int32_t subcmd_found_idx = -1;
  int32_t cmd_end_pos = argc;

  for (int i = argc - 1; i > 0; i--) {
    if constexpr (!is_same_v<SubParsers, tuple<>>) {
      if (subcmd_found_idx == -1) {
        subcmd_found_idx = ParserIndex(subParsers, argv[i]);
        if (subcmd_found_idx != -1) {
          cmd_end_pos = i;
        }
      }
    }
  }
  bool flag = false;
  string_view arg;

  for (int i = 1; i < cmd_end_pos + 1; i++) {
    if (i != cmd_end_pos) {
      arg = argv[i];
      flag = arg.starts_with('-');
    } else {
      flag = true;
    }
    if (i != 1 and flag) {
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
        } else {
          throw InvalidArgument("No keys specified");
        }
      }
    }
    if (i == cmd_end_pos) {
      break;
    }
    if (flag) {
      if (arg.size() > 1 and arg.at(1) == '-') {
        if (arg.contains('=')) [[unlikely]] {
          auto equal_pos = arg.find('=');
          key = arg.substr(2, equal_pos - 2);
          values.push_back(arg.substr(equal_pos + 1));
          flag = true;
        } else {
          key = arg.substr(2);
        }
      } else {
        for (const auto& j : arg.substr(1)) {
          short_keys.push_back(j);
        }
      }
    } else {
      values.push_back(arg);
    }
  }

  auto required_keys = vector<string_view>();
  tuple_type_visit<decltype(tuple_cat(declval<Args>(), declval<PArgs>()))>(
      [&required_keys]<class T>(T) {
        if ((T::type::required && !T::type::assigned)) {
          required_keys.push_back(string_view(T::type::name));
        }
      });

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

inline constexpr auto createUsageSection(const auto& program_name,
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

inline constexpr auto createSubcommandSection(const auto& ansi,
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

inline constexpr auto createOptionsSection(const auto& ansi,
                                           const auto& help_info) {
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
inline constexpr auto createPositionalArgumentSection(const auto& ansi) {
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
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::formatHelp(
    bool no_color) const -> string {
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

