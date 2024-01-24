#pragma once

#include <unistd.h>

#include <array>
#include <cassert>
#include <charconv>
#include <concepts>
#include <cstring>
#include <filesystem>
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

class ParserInternalError : public std::runtime_error {
 public:
  explicit ParserInternalError(const std::string& msg) : runtime_error(msg) {}

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return runtime_error::what();
  }
};

class ParseError : public std::runtime_error {
 public:
  explicit ParseError(const std::string& msg) : runtime_error(msg) {}

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return runtime_error::what();
  }
};

/*!
 * InvalidArgument exception class
 */
class InvalidArgument : public std::invalid_argument {
 public:
  explicit InvalidArgument(const std::string& msg) : invalid_argument(msg) {}

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return invalid_argument::what();
  }
};

class ValidationError : public InvalidArgument {
 public:
  explicit ValidationError(const std::string& msg) : InvalidArgument(msg) {}

  [[nodiscard]] auto what() const noexcept -> const char* override {
    return InvalidArgument::what();
  }
};

}  // namespace Argo


namespace Argo {

template <class T>
struct is_tuple : std::false_type {};

template <class... T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

template <class T>
constexpr bool is_tuple_v = is_tuple<T>::value;

template <class T>
struct is_vector : std::false_type {};

template <class T>
struct is_vector<std::vector<T>> : std::true_type {};

template <class T>
constexpr bool is_vector_v = is_vector<T>::value;

template <class T>
struct vector_base {
  using type = T;
};

template <class T>
struct vector_base<std::vector<T>> {
  using type = T;
};

template <class T>
using vector_base_t = vector_base<T>::type;

template <class T>
struct is_array : std::false_type {};

template <class T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type {};

template <class T>
constexpr bool is_array_v = is_array<T>::value;

template <class T>
struct array_len {
  static constexpr std::size_t value = 0;
};

template <class T, std::size_t N>
struct array_len<std::array<T, N>> {
  static constexpr std::size_t value = N;
};

template <class T>
constexpr std::size_t array_len_v = array_len<T>::value;

template <class T>
struct array_base {
  using type = T;
};

template <class T, std::size_t N>
struct array_base<std::array<T, N>> {
  using type = T;
};

template <class T>
using array_base_t = array_base<T>::type;

template <class T, class... U>
struct tuple_append {
  using type = std::tuple<U..., T>;
};

template <class T, class... U>
struct tuple_append<std::tuple<U...>, T> {
  using type = std::tuple<U..., T>;
};

template <class... T>
using tuple_append_t = typename tuple_append<T...>::type;

template <class... T>
struct type_sequence {};

template <class T>
struct make_type_sequence {};

template <class... T>
struct make_type_sequence<std::tuple<T...>> {
  using type = type_sequence<T...>;
};

template <class T>
using make_type_sequence_t = make_type_sequence<T>::type;

template <class Tuple, class T>
ARGO_ALWAYS_INLINE constexpr auto tuple_type_visit(T fun) {
  [&fun]<class... U>(type_sequence<U...>) ARGO_ALWAYS_INLINE {
    (fun(std::type_identity<U>()), ...);
  }(make_type_sequence_t<Tuple>());
}

template <class Tuple, class T>
ARGO_ALWAYS_INLINE constexpr auto tuple_type_or_visit(T fun) -> bool {
  return [&fun]<class... U>(type_sequence<U...>) ARGO_ALWAYS_INLINE {
    return (fun(std::type_identity<U>()) || ...);
  }(make_type_sequence_t<Tuple>());
}

template <class Tuple, class T>
ARGO_ALWAYS_INLINE constexpr auto tuple_type_and_visit(T fun) -> bool {
  return [&fun]<class... U>(type_sequence<U...>) ARGO_ALWAYS_INLINE {
    return (fun(std::type_identity<U>()) && ...);
  }(make_type_sequence_t<Tuple>());
}

};  // namespace Argo


namespace Argo::Validation {

struct ValidationBase {
  template <class T>
  auto operator()(const T& value, std::span<std::string_view> values,
                  std::string_view option_name) -> void {
    if (!this->isValid(value, values)) [[unlikely]] {
      throw ValidationError(
          format("Option {} has invalid value {}", option_name, value));
    }
  }

  template <class T>
  auto operator()(const T& value, std::span<std::string_view> raw_val) {
    return this->isValid(value, raw_val);
  }

  template <class T>
  [[noreturn]] auto isValid(const T& /* unused */,
                            std::span<std::string_view> /* unuesd */) const
      -> bool {
    static_assert(false, "Invalid validation");
  };

  ValidationBase() = default;
  ValidationBase(const ValidationBase&) = default;
  ValidationBase(ValidationBase&&) = default;
  auto operator=(const ValidationBase&) -> ValidationBase& = default;
  auto operator=(ValidationBase&&) -> ValidationBase& = default;
  virtual ~ValidationBase() = default;
};

template <std::derived_from<ValidationBase> Lhs,
          std::derived_from<ValidationBase> Rhs>
struct AndValidation : ValidationBase {
 private:
  Lhs lhs_;
  Rhs rhs_;

 public:
  AndValidation(Lhs lhs, Rhs rhs) : lhs_(lhs), rhs_(rhs){};

  template <class U>
  auto isValid(const U& value, std::span<std::string_view> raw_values) const
      -> bool {
    return this->lhs_(value, raw_values) && this->lhs_(value, raw_values);
  };
};

template <std::derived_from<ValidationBase> Lhs,
          std::derived_from<ValidationBase> Rhs>
AndValidation(Lhs, Rhs) -> AndValidation<Lhs, Rhs>;

template <std::derived_from<ValidationBase> Lhs,
          std::derived_from<ValidationBase> Rhs>
struct OrValidation : ValidationBase {
 private:
  Lhs lhs_;
  Rhs rhs_;

 public:
  OrValidation(Lhs lhs, Rhs rhs) : lhs_(lhs), rhs_(rhs){};

  template <class U>
  auto isValid(const U& value, std::span<std::string_view> raw_values) const
      -> bool {
    return this->lhs_(value, raw_values) || this->lhs_(value, raw_values);
  };
};

template <std::derived_from<ValidationBase> Lhs,
          std::derived_from<ValidationBase> Rhs>
OrValidation(Lhs, Rhs) -> OrValidation<Lhs, Rhs>;

template <std::derived_from<ValidationBase> Rhs>
struct InvertValidation : ValidationBase {
 private:
  Rhs rhs_;

 public:
  explicit InvertValidation(Rhs rhs) : rhs_(rhs){};

  template <class U>
  auto isValid(const U& value, std::span<std::string_view> raw_values) const
      -> bool {
    return !this->lhs_(value, raw_values);
  };
};

template <std::derived_from<ValidationBase> Rhs>
InvertValidation(Rhs) -> InvertValidation<Rhs>;

template <class T>
struct Range final : public ValidationBase {
 private:
  T min_;
  T max_;

 public:
  Range(T min, T max) : min_(min), max_(max){};

  template <class U>
  auto operator()(const U& value, std::span<std::string_view> values,
                  std::string_view option_name) -> void {
    if (!this->isValid(value, values)) [[unlikely]] {
      throw ValidationError(
          format("Option {} has invalid value {}", option_name, value));
    }
  }

  template <class U>
  auto isValid(const U& value, std::span<std::string_view> /* unused */) const
      -> bool {
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

/*!
 * ArgName which holds argument name
 */
template <std::size_t N>
struct ArgName {
  char short_key_ = '\0';
  char key_[N] = {};
  std::size_t key_len_ = N;

  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval ArgName(const char (&lhs)[N + 1]) {
    for (std::size_t i = 0; i < N; i++) {
      if (lhs[i] == ',') {
        this->key_len_ = i;
        this->short_key_ = lhs[i + 1];
        return;
      }
      this->key_[i] = lhs[i];
    }
  };

  [[nodiscard]] ARGO_ALWAYS_INLINE constexpr auto getShortName() const {
    return this->short_key_;
  }

  [[nodiscard]] ARGO_ALWAYS_INLINE constexpr auto getKey() const {
    return std::string_view(this->key_, this->key_len_);
  }

  [[nodiscard]] constexpr auto getKeyLen() const {
    return this->key_len_;
  }

  template <std::size_t M>
  [[nodiscard]] ARGO_ALWAYS_INLINE consteval auto operator==(
      const ArgName<M>& lhs) const -> bool {
    return this->getKey() == lhs.getKey();
  }

  [[nodiscard]] ARGO_ALWAYS_INLINE consteval auto hasValidNameLength() const
      -> bool {
    if (this->getShortName() == '\0') {
      return true;
    }
    return (N - this->key_len_) == 2;
  }
};

template <std::size_t N>
ArgName(const char (&)[N]) -> ArgName<N - 1>;

template <class T>
concept ArgNameType = requires(T& x) {
  std::is_same_v<decltype(x.getKey()), std::string_view>;
  std::is_same_v<decltype(x.getShortName()), char>;
};

}  // namespace Argo


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


namespace Argo {

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
  std::string_view description;
};

ARGO_ALWAYS_INLINE constexpr auto description(std::string_view desc)
    -> Description {
  return {.description = desc};
}

template <class T>
ARGO_ALWAYS_INLINE constexpr auto explicitDefault(T value)
    -> ExlicitDefaultValue<T> {
  return {.explicit_default_value = value};
}

template <class T>
ARGO_ALWAYS_INLINE constexpr auto implicitDefault(T value)
    -> ImplicitDefaultValue<T> {
  return {.implicit_default_value = value};
}

template <class Type, ArgName Name, NArgs nargs, bool Required, ParserID ID,
          class... Args>
ARGO_ALWAYS_INLINE constexpr auto ArgInitializer(Args... args) -> void {
  (
      [&args]() ARGO_ALWAYS_INLINE {
        using Arg = Arg<Type, Name, nargs, Required, ID>;
        if constexpr (std::is_same_v<Args, Description>) {
          Arg::description = args.description;
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               Validation::ValidationBase>) {
          static_assert(std::is_invocable_v<Args, typename Arg::type,
                                            std::span<std::string_view>,
                                            std::string_view>,
                        "Invalid validator");
          Arg::validator = args;
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               ImplicitDefaultValueTag>) {
          Arg::defaultValue = static_cast<Type>(args.implicit_default_value);
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               ExplicitDefaultValueTag>) {
          Arg::value = static_cast<Type>(args.explicit_default_value);
        } else if constexpr (std::is_invocable_v<Args, typename Arg::type&,
                                                 std::span<std::string_view>>) {
          Arg::callback = args;
        } else {
          static_assert(false, "Invalid argument");
        }
      }(),
      ...);
}

template <ArgName Name, ParserID ID, class... Args>
ARGO_ALWAYS_INLINE constexpr auto FlagArgInitializer(Args... args) -> void {
  (
      [&args]() ARGO_ALWAYS_INLINE {
        using FlagArg = FlagArg<Name, ID>;
        if constexpr (std::is_same_v<Args, Description>) {
          FlagArg::description = args.description;
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               Validation::ValidationBase>) {
          static_assert(false, "Flag cannot have validator");
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               ImplicitDefaultValueTag>) {
          static_assert(false, "Flag cannot have implicit default value");
        } else if constexpr (std::derived_from<std::remove_cvref_t<Args>,
                                               ExplicitDefaultValueTag>) {
          static_assert(false, "Flag cannot have explicit default value");
        } else if constexpr (std::is_invocable_v<Args>) {
          FlagArg::callback = args;
        } else {
          static_assert(false, "Invalid argument");
        }
      }(),
      ...);
}

};  // namespace Argo


namespace Argo {

struct ArgInfo {
  std::string_view name;
  char shortName;
  std::string_view description;
  bool required;
  std::string_view typeName;
};

template <class Args>
ARGO_ALWAYS_INLINE constexpr auto HelpGenerator() {
  std::vector<ArgInfo> ret;
  tuple_type_visit<Args>([&ret]<class T>(T) {
    if constexpr (std::derived_from<typename T::type, ArgTag>) {
      ret.emplace_back(
          T::type::name.getKey().substr(0, T::type::name.getKeyLen()),  //
          T::type::name.getShortName(),                                 //
          T::type::description,                                         //
          T::type::required,                                            //
          std::string_view(T::type::typeName));
    } else {
      ret.emplace_back(
          T::type::name.getKey().substr(0, T::type::name.getKeyLen()),  //
          T::type::name.getShortName(),                                 //
          T::type::description,                                         //
          false,                                                        //
          std::string_view(T::type::typeName));
    }
  });
  return ret;
};

struct SubCommandInfo {
  std::string_view name;
  std::string_view description;
};

template <class T>
ARGO_ALWAYS_INLINE constexpr auto SubParserInfo(T subparsers) {
  std::vector<SubCommandInfo> ret{};
  if constexpr (!std::is_same_v<T, std::tuple<>>) {
    apply(
        [&ret]<class... Parser>(Parser... parser) ARGO_ALWAYS_INLINE {
          (..., ret.emplace_back(parser.name.getKey(), parser.description));
        },
        subparsers);
  }
  return ret;
};

}  // namespace Argo


namespace Argo {

template <class Arguments>
ARGO_ALWAYS_INLINE constexpr auto GetkeyFromShortKey(char key)
    -> std::tuple<std::string_view, bool> {
  auto name = std::string_view();
  auto is_flag = true;
  if ([&]<class... T>(type_sequence<T...>) ARGO_ALWAYS_INLINE {
        return ([&] {
          if (T::name.getShortName() == key) {
            name = T::name.getKey();
            if constexpr (!std::derived_from<T, FlagArgTag>) {
              is_flag = false;
            }
            return true;
          }
          return false;
        }() || ...);
      }(make_type_sequence_t<Arguments>())) [[likely]] {
    return make_tuple(name, is_flag);
  }
  throw ParserInternalError("Fail to lookup");
}

/*!
 * Index Search meta function
 */
template <class Tuple, ArgName Name>
consteval auto SearchIndex() -> int {
  int value = -1;
  if ([&value]<class... T>(type_sequence<T...>) ARGO_ALWAYS_INLINE {
        return ((value++, Name == T::name) || ...);
      }(make_type_sequence_t<Tuple>())) {
    return value;
  }
  return -1;
}

/*!
 * Index Search meta function
 */
template <class Tuple, char C>
consteval auto SearchIndexFromShortName() -> int {
  int value = -1;
  if ([&value]<class... T>(type_sequence<T...>) ARGO_ALWAYS_INLINE {
        return ((value++, C == T::name.getShortName()) || ...);
      }(make_type_sequence_t<Tuple>())) {
    return value;
  }
  return -1;
}

/*!
 * Index Search meta function
 */
template <class Tuple>
ARGO_ALWAYS_INLINE constexpr auto IsFlag(char c) -> bool {
  return [&c]<class... T>(type_sequence<T...>) ARGO_ALWAYS_INLINE {
    return ((c == T::name.getShortName()) || ...);
  }(make_type_sequence_t<Tuple>());
}

};  // namespace Argo


namespace Argo {

/*!
 * Helper class of assigning value
 */
template <class Type>
ARGO_ALWAYS_INLINE constexpr auto ArgCaster(const std::string_view& value,
                                            const std::string_view& key)
    -> Type {
  if constexpr (std::is_same_v<Type, bool>) {
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
    throw InvalidArgument(
        std::format("Argument {}: {} cannot convert bool", key, value));
  } else if constexpr (std::is_integral_v<Type>) {
    Type ret;
    std::from_chars(value.begin(), value.end(), ret);
    return ret;
  } else if constexpr (std::is_floating_point_v<Type>) {
    return static_cast<Type>(std::stod(std::string(value)));
  } else if constexpr (std::is_same_v<Type, const char*>) {
    return value.data();
  } else {
    return static_cast<Type>(value);
  }
}

template <class... T, std::size_t... N>
ARGO_ALWAYS_INLINE constexpr auto TupleAssign(
    std::tuple<T...>& t, const std::span<std::string_view>& v,
    std::index_sequence<N...> /* unused */, const std::string_view& key)
    -> void {
  ((get<N>(t) = ArgCaster<std::remove_cvref_t<decltype(get<N>(t))>>(v[N], key)),
   ...);
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto AfterAssign(
    const std::span<std::string_view>& values) -> void {
  Arg::assigned = true;
  if (Arg::validator) {
    Arg::validator(Arg::value, values, Arg::name.getKey());
  }
  if (Arg::callback) {
    Arg::callback(Arg::value, values);
  }
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto ValiadicArgAssign(
    const std::span<std::string_view>& values) -> void {
  Arg::value.resize(values.size());
  for (std::size_t i = 0; i < values.size(); i++) {
    Arg::value[i] =
        ArgCaster<typename Arg::baseType>(values[i], Arg::name.getKey());
  }
  AfterAssign<Arg>(values);
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto NLengthArgAssign(
    std::span<std::string_view>& values) -> void {
  if (Arg::nargs.nargs > values.size()) [[unlikely]] {
    throw Argo::InvalidArgument(std::format("Argument {}: invalid argument {}",
                                            Arg::name.getKey(), values));
  }
  if constexpr (is_array_v<typename Arg::type>) {
    for (std::size_t i = 0; i < Arg::nargs.nargs; i++) {
      Arg::value[i] =
          ArgCaster<typename Arg::baseType>(values[i], Arg::name.getKey());
    }
  } else if constexpr (is_vector_v<typename Arg::type>) {
    Arg::value.resize(Arg::nargs.nargs);
    for (std::size_t i = 0; i < Arg::nargs.nargs; i++) {
      Arg::value[i] =
          ArgCaster<typename Arg::baseType>(values[i], Arg::name.getKey());
    }
  } else if constexpr (is_tuple_v<typename Arg::type>) {
    TupleAssign(
        Arg::value, values,
        std::make_index_sequence<std::tuple_size_v<typename Arg::type>>(),
        Arg::name.getKey());
  } else {
    static_assert(false, "Invalid Type");
  }
  AfterAssign<Arg>(values.subspan(0, Arg::nargs.nargs));
  values = values.subspan(Arg::nargs.nargs);
}

template <class Arg>
ARGO_ALWAYS_INLINE constexpr auto ZeroOrOneArgAssign(
    std::span<std::string_view>& values) -> void {
  if (values.empty()) {
    Arg::value = Arg::defaultValue;
  } else {
    Arg::value = ArgCaster<typename Arg::type>(values[0], Arg::name.getKey());
  }
  AfterAssign<Arg>(values.subspan(0, 1));
  values = values.subspan(1);
}

template <class PArgs>
ARGO_ALWAYS_INLINE constexpr auto PArgAssigner(
    std::span<std::string_view> values) -> bool {
  return [&values]<class... Arg>(type_sequence<Arg...>) ARGO_ALWAYS_INLINE {
    return ([&values] ARGO_ALWAYS_INLINE {
      if (Arg::assigned) {
        return false;
      }
      if constexpr (Arg::nargs.nargs_char == '+') {
        ValiadicArgAssign<Arg>(values);
        return true;
      }
      if constexpr (Arg::nargs.nargs == 1) {
        if (values.empty()) [[unlikely]] {
          throw Argo::InvalidArgument(
              std::format("Argument {}: should take exactly one value but zero",
                          Arg::name.getKey()));
        }
        ZeroOrOneArgAssign<Arg>(values);
        return values.empty();
      }
      if constexpr (Arg::nargs.nargs > 1) {
        NLengthArgAssign<Arg>(values);
        return values.empty();
      }
    }() || ...);
  }(make_type_sequence_t<PArgs>());
}

template <>
ARGO_ALWAYS_INLINE constexpr auto PArgAssigner<std::tuple<>>(
    std::span<std::string_view> /*unused*/) -> bool {
  return true;
}

template <class Head, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto AssignOneArg(
    const std::string_view& key, std::span<std::string_view> values) -> bool {
  if (Head::assigned) [[unlikely]] {
    throw Argo::InvalidArgument(
        std::format("Argument {}: duplicated argument", key));
  }
  if constexpr (std::derived_from<Head, FlagArgTag>) {
    if constexpr (std::is_same_v<PArgs, std::tuple<>>) {
      if (!values.empty()) [[unlikely]] {
        throw Argo::InvalidArgument(
            std::format("Flag {} can not take value", key));
      }
    } else {
      if (!values.empty()) {
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
    if constexpr (Head::nargs.nargs_char == '?') {
      ZeroOrOneArgAssign<Head>(values);
      if (values.empty()) {
        return true;
      }
      return PArgAssigner<PArgs>(values);
    } else if constexpr (Head::nargs.nargs_char == '*') {
      if (values.empty()) {
        Head::value = Head::defaultValue;
        Head::assigned = true;
        return true;
      }
      ValiadicArgAssign<Head>(values);
      return true;
    } else if constexpr (Head::nargs.nargs_char == '+') {
      if (values.empty()) [[unlikely]] {
        throw Argo::InvalidArgument(
            std::format("Argument {}: should take more than one value", key));
      }
      ValiadicArgAssign<Head>(values);
      return true;
    } else if constexpr (Head::nargs.nargs == 1) {
      if (values.empty()) [[unlikely]] {
        throw Argo::InvalidArgument(std::format(
            "Argument {}: should take exactly one value but zero", key));
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
ARGO_ALWAYS_INLINE constexpr auto assignArg(
    const std::string_view& key, const std::span<std::string_view>& values) {
  [&key, &values]<std::size_t... Is>(std::index_sequence<Is...>)
      ARGO_ALWAYS_INLINE -> void {
        if (!(... || (std::tuple_element_t<Is, Args>::name.getKey() == key and
                      AssignOneArg<std::tuple_element_t<Is, Args>, PArgs>(
                          key, values)))) [[unlikely]] {
          throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
        }
      }(std::make_index_sequence<std::tuple_size_v<Args>>());
}

template <class Arguments, class PArgs>
ARGO_ALWAYS_INLINE constexpr auto Assigner(
    std::string_view key, const std::span<std::string_view>& values) -> void {
  if constexpr (!std::is_same_v<PArgs, std::tuple<>>) {
    if (key.empty()) {
      if (!PArgAssigner<PArgs>(values)) [[unlikely]] {
        throw InvalidArgument("Duplicated positional argument");
      }
      return;
    }
  } else {
    if (key.empty()) [[unlikely]] {
      throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
    }
  }
  assignArg<Arguments, PArgs>(key, values);
}

template <class Arguments, class PArgs, class HArg>
ARGO_ALWAYS_INLINE constexpr auto ShortArgAssigner(
    std::string_view key, const std::span<std::string_view>& values) {
  for (std::size_t i = 0; i < key.size(); i++) {
    auto [found_key, is_flag] = GetkeyFromShortKey<     //
        std::conditional_t<std::is_same_v<HArg, void>,  //
                           Arguments,                   //
                           tuple_append_t<Arguments, HArg>>>(key[i]);
    if constexpr (!std::is_same_v<HArg, void>) {
      if (found_key == HArg::name.getKey()) [[unlikely]] {
        return true;
      }
    }
    if (is_flag and (key.size() - 1 == i) and !values.empty()) {
      assignArg<Arguments, PArgs>(found_key, values);
    } else if (is_flag) {
      assignArg<Arguments, PArgs>(found_key, {});
    } else if ((key.size() - 1 == i) and !values.empty()) {
      assignArg<Arguments, PArgs>(found_key, values);
      return false;
    } else if ((key.size() - 1 == i) and values.empty()) {
      auto value = std::vector<std::string_view>{key.substr(i + 1)};
      assignArg<Arguments, PArgs>(found_key, value);
      return false;
    } else [[unlikely]] {
      throw Argo::InvalidArgument(std::format("Invalid Flag argument {} {}",
                                              key[i], key.substr(i + 1)));
    }
  }
  return false;
}

template <class Args>
ARGO_ALWAYS_INLINE constexpr auto ValueReset() -> void {
  []<std::size_t... Is>(std::index_sequence<Is...>) ARGO_ALWAYS_INLINE {
    (..., []<class T>() ARGO_ALWAYS_INLINE {
      if (T::assigned) {
        T::value = typename T::type();
        T::assigned = false;
      }
    }.template operator()<std::tuple_element_t<Is, Args>>());
  }(std::make_index_sequence<std::tuple_size_v<Args>>());
}

};  // namespace Argo


namespace Argo {

template <ArgName Name, class Parser>
struct SubParser {
  static constexpr auto name = Name;
  std::reference_wrapper<Parser> parser;
  std::string_view description;
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
ARGO_ALWAYS_INLINE constexpr auto MetaParse(SubParsers sub_parsers, int index,
                                            int argc, char** argv) -> bool {
  return apply(
      [&](auto&&... s) ARGO_ALWAYS_INLINE {
        std::int64_t idx = -1;
        return (... || (idx++, idx == index &&
                                   (s.parser.get().parse(argc, argv), true)));
      },
      sub_parsers);
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
ARGO_ALWAYS_INLINE constexpr auto ParserIndex(SubParsers sub_parsers,  //
                                              std::string_view key)
    -> std::int64_t {
  return apply(
      [&](auto&&... s) ARGO_ALWAYS_INLINE {
        std::int64_t index = -1;
        bool found = (... || (index++, s.name.getKey() == key));
        return found ? index : -1;
      },
      sub_parsers);
};

}  // namespace Argo


namespace Argo {

struct Unspecified {};

enum class RequiredFlag : bool {
  Optional = false,
  Required = true,
};

using RequiredFlag::Required;  // NOLINT(misc-unused-using-decls)
using RequiredFlag::Optional;  // NOLINT(misc-unused-using-decls)

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
  std::optional<std::string_view> help = std::nullopt;
  std::optional<std::string_view> program_name = std::nullopt;
  std::optional<std::string_view> description = std::nullopt;
  std::optional<std::string_view> usage = std::nullopt;
  std::optional<std::string_view> subcommand_help = std::nullopt;
  std::optional<std::string_view> options_help = std::nullopt;
  std::optional<std::string_view> positional_argument_help = std::nullopt;
};

template <ParserID ID = 0, class Args = std::tuple<>,
                 class PArgs = std::tuple<>, class HArg = void,
                 class SubParsers = std::tuple<>>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
class Parser {
 private:
  bool parsed_ = false;
  std::unique_ptr<ParserInfo> info_ = nullptr;
  SubParsers subParsers;

 public:
  constexpr explicit Parser() : info_(std::make_unique<ParserInfo>()){};

  constexpr explicit Parser(std::string_view program_name)
      : info_(std::make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
  };

  constexpr explicit Parser(std::string_view program_name,
                            std::string_view description)
      : info_(std::make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
    this->info_->description = description;
  };

  constexpr explicit Parser(std::string_view program_name,
                            Argo::Description description)
      : info_(std::make_unique<ParserInfo>()) {
    this->info_->program_name = program_name;
    this->info_->description = description.description;
  };

  constexpr explicit Parser(SubParsers tuple) : subParsers(tuple) {}

  constexpr explicit Parser(std::unique_ptr<ParserInfo> info, SubParsers tuple)
      : info_(std::move(info)), subParsers(tuple){};

  Parser(const Parser&) = delete;
  Parser(Parser&&) = delete;

  constexpr auto operator=(const Parser&) -> Parser& = delete;
  constexpr auto operator=(Parser&&) -> Parser& = delete;
  constexpr ~Parser() = default;

  template <class Type, ArgName Name, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), bool ISPArgs, class... T>
  ARGO_ALWAYS_INLINE constexpr auto createArg(T... args) {
    static_assert(Name.hasValidNameLength(),
                  "Short name can't be more than one charactor");

    static constexpr auto nargs = []() {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg1)>,
                                   NArgs>) {
        if constexpr (is_array_v<Type>) {
          static_assert(array_len_v<Type> == arg1.nargs,
                        "Array size mismatch with nargs");
        }
        if constexpr (is_vector_v<Type>) {
          static_assert(arg1.nargs_char != '?' && arg1.nargs != 1,
                        "Vector size mismatch with nargs");
        }
        if constexpr (is_tuple_v<Type>) {
          static_assert(std::tuple_size_v<Type> == arg1.nargs,
                        "Tuple size mismatch with nargs");
        }
        return arg1;
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg2)>,
                                          NArgs>) {
        if constexpr (is_array_v<Type>) {
          static_assert(array_len_v<Type> == arg2.nargs,
                        "Array size mismatch with nargs");
        }
        if constexpr (is_vector_v<Type>) {
          static_assert(arg2.nargs_char != '?' && arg2.nargs != 1,
                        "Vector size mismatch with nargs");
        }
        if constexpr (is_tuple_v<Type>) {
          static_assert(std::tuple_size_v<Type> == arg2.nargs,
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
          return NArgs{static_cast<int>(std::tuple_size_v<Type>)};
        }
        if constexpr (ISPArgs) {
          return NArgs(1);
        }
        return NArgs('?');
      }
    }();

    static_assert(!(is_array_v<Type> and nargs.nargs == 1),
                  "Array size must be more than one");
    static_assert(!(is_tuple_v<Type> and nargs.nargs == 1),
                  "Tuple size must be more than one");
    static constexpr auto required = []() {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg1)>,
                                   RequiredFlag>) {
        return static_cast<bool>(arg1);
      } else if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg2)>,
                                          RequiredFlag>) {
        return static_cast<bool>(arg2);
      } else {
        return false;
      }
    }();

    if constexpr (!std::is_same_v<PArgs, std::tuple<>>) {
      static_assert(SearchIndex<PArgs, Name>() == -1, "Duplicated name");
    }
    static_assert(
        (Name.getShortName() == '\0') ||
            (SearchIndexFromShortName<Args, Name.getShortName()>() == -1),
        "Duplicated short name");
    static_assert(SearchIndex<Args, Name>() == -1, "Duplicated name");
    static_assert(                     //
        (nargs.nargs > 0               //
         || nargs.nargs_char == '?'    //
         || nargs.nargs_char == '+'    //
         || nargs.nargs_char == '*'),  //
        "nargs must be '?', '+', '*' or int");

    ArgInitializer<Type, Name, nargs, required, ID>(std::forward<T>(args)...);
    return std::type_identity<Arg<Type, Name, nargs, required, ID>>();
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
    static_assert(Name.getShortName() == '\0',
                  "Positional argment cannot have short name");
    auto arg =
        createArg<Type, Name, arg1, arg2, true>(std::forward<T>(args)...);

    static_assert(decltype(arg)::type::nargs.nargs_char != '?',
                  "Cannot assign narg: ? to the positional argument");
    static_assert(decltype(arg)::type::nargs.nargs_char != '*',
                  "Cannot assign narg: * to the positional argument");

    return Parser<ID, Args, tuple_append_t<PArgs, typename decltype(arg)::type>,
                  HArg, SubParsers>(std::move(this->info_), subParsers);
  }

  template <ArgName Name, class... T>
  constexpr auto addFlag(T... args) {
    if constexpr (!std::is_same_v<PArgs, std::tuple<>>) {
      static_assert(SearchIndex<PArgs, Name>() == -1, "Duplicated name");
    }
    static_assert(
        (Name.getShortName() == '\0') ||
            (SearchIndexFromShortName<Args, Name.getShortName()>() == -1),
        "Duplicated short name");
    static_assert(SearchIndex<Args, Name>() == -1, "Duplicated name");
    FlagArgInitializer<Name, ID>(std::forward<T>(args)...);
    return Parser<ID, tuple_append_t<Args, FlagArg<Name, ID>>, PArgs, HArg,
                  SubParsers>(std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  constexpr auto addHelp() {
    static_assert((SearchIndexFromShortName<Args, Name.getShortName()>() == -1),
                  "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>() == -1, "Duplicated name");
    return Parser<ID, Args, PArgs, HelpArg<Name, ID>, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  constexpr auto addHelp(std::string_view help) {
    static_assert((SearchIndexFromShortName<Args, Name.getShortName()>() == -1),
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
    if (!this->parsed_) [[unlikely]] {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!std::is_same_v<PArgs, std::tuple<>>) {
      if constexpr (SearchIndex<PArgs, Name>() != -1) {
        return std::tuple_element_t<SearchIndex<PArgs, Name>(), PArgs>::value;
      } else {
        static_assert(SearchIndex<Args, Name>() != -1,
                      "Argument does not exist");
        return std::remove_cvref_t<decltype(get<SearchIndex<Args, Name>()>(
            std::declval<Args>()))>::value;
      }
    } else {
      static_assert(SearchIndex<Args, Name>() != -1, "Argument does not exist");
      return std::remove_cvref_t<decltype(get<SearchIndex<Args, Name>()>(
          std::declval<Args>()))>::value;
    }
  }

  template <ArgName Name>
  constexpr auto getParser() -> auto& {
    if constexpr (std::is_same_v<SubParsers, std::tuple<>>) {
      static_assert(false, "Parser has no sub parser");
    }
    static_assert(!(SearchIndex<SubParsers, Name>() == -1),
                  "Could not find subparser");
    return get<SearchIndex<SubParsers, Name>()>(subParsers).parser.get();
  }

  template <ArgName Name>
  constexpr auto isAssigned() {
    if (!this->parsed_) [[unlikely]] {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    using AllArguments =
        decltype(std::tuple_cat(std::declval<Args>(), std::declval<PArgs>()));

    static_assert(SearchIndex<AllArguments, Name>() != -1,
                  "Argument does not exist");

    return std::tuple_element_t<SearchIndex<AllArguments, Name>(),
                                AllArguments>::assigned;
  }

  /*!
   * Add subcommand
   */
  template <ArgName Name, class T>
  ARGO_ALWAYS_INLINE constexpr auto addParser(T& sub_parser,
                                              Description description = {""}) {
    auto s = make_tuple(
        SubParser<Name, T>{ref(sub_parser), description.description});
    auto sub_parsers = std::tuple_cat(subParsers, s);
    return Parser<ID, Args, PArgs, HArg, decltype(sub_parsers)>(
        std::move(this->info_), sub_parsers);
  }

  ARGO_ALWAYS_INLINE constexpr auto resetArgs() -> void;

  ARGO_ALWAYS_INLINE constexpr auto addUsageHelp(std::string_view usage) {
    this->info_->usage = usage;
  }

  ARGO_ALWAYS_INLINE constexpr auto addSubcommandHelp(
      std::string_view subcommand_help) {
    this->info_->subcommand_help = subcommand_help;
  }

  ARGO_ALWAYS_INLINE constexpr auto addPositionalArgumentHelp(
      std::string_view positional_argument_help) {
    this->info_->positional_argument_help = positional_argument_help;
  }

  ARGO_ALWAYS_INLINE constexpr auto addOptionsHelp(
      std::string_view options_help) {
    this->info_->options_help = options_help;
  }

 private:
  ARGO_ALWAYS_INLINE constexpr auto setArg(
      std::string_view key, const std::span<std::string_view>& val) const
      -> void;
  ARGO_ALWAYS_INLINE constexpr auto setShortKeyArg(
      std::string_view short_key, const std::span<std::string_view>& val) const
      -> void;

 public:
  ARGO_ALWAYS_INLINE constexpr auto parse(int argc, char* argv[]) -> void;
  [[nodiscard]] constexpr auto formatHelp(bool no_color = false) const
      -> std::string;

  explicit constexpr operator bool() const {
    return this->parsed_;
  }
};

}  // namespace Argo


namespace Argo {

ARGO_ALWAYS_INLINE auto splitStringView(std::string_view str, char delimeter)
    -> std::vector<std::string_view> {
  std::vector<std::string_view> ret;
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
  this->parsed_ = false;
  ValueReset<decltype(std::tuple_cat(std::declval<Args>(),
                                     std::declval<PArgs>()))>();
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::setArg(
    std::string_view key, const std::span<std::string_view>& val) const
    -> void {
  if constexpr (!std::is_same_v<HArg, void>) {
    if (key == HArg::name.getKey()) {
      std::cout << formatHelp() << std::endl;
      std::exit(0);
    }
  }
  Assigner<Args, PArgs>(key, val);
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::setShortKeyArg(
    std::string_view key, const std::span<std::string_view>& val) const
    -> void {
  if (ShortArgAssigner<Args, PArgs, HArg>(key, val)) [[unlikely]] {
    std::cout << formatHelp() << std::endl;
    std::exit(0);
  };
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::parse(int argc,
                                                                char* argv[])
    -> void {
  if (this->parsed_) [[unlikely]] {
    throw ParseError("Cannot parse twice");
  }
  auto assigned_keys = std::vector<std::string_view>();
  tuple_type_visit<decltype(std::tuple_cat(std::declval<Args>(),
                                           std::declval<PArgs>()))>(
      [&assigned_keys]<class T>(T) {
        if (T::type::assigned) {
          assigned_keys.push_back(T::type::name.getKey());
        }
      });
  if (!assigned_keys.empty()) [[unlikely]] {
    throw ParseError(std::format("keys {} already assigned", assigned_keys));
  }

  std::string_view key{};
  std::string_view short_keys{};
  std::vector<std::string_view> values{};

  // [[assume(this->info_)]]; // TODO(gen740): add assume when clang supports it

  if (!this->info_->program_name) {
    this->info_->program_name = std::string_view(argv[0]);
  }

  // Search for subcommand
  std::int32_t subcmd_found_idx = -1;
  std::int32_t cmd_end_pos = argc;

  for (int i = argc - 1; i > 0; i--) {
    if constexpr (!std::is_same_v<SubParsers, std::tuple<>>) {
      if (subcmd_found_idx == -1) {
        subcmd_found_idx = ParserIndex(subParsers, argv[i]);
        if (subcmd_found_idx != -1) {
          cmd_end_pos = i;
        }
      }
    }
  }
  bool is_flag = false;
  std::string_view arg;

  for (int i = 1; i < cmd_end_pos + 1; i++) {
    if (i != cmd_end_pos) {
      arg = argv[i];
      is_flag = arg.starts_with('-');
      if (arg.size() > 1 and arg.at(1) >= '0' and arg.at(1) <= '9') {
        is_flag = IsFlag<Args>(arg.at(1));
      }
    } else {
      is_flag = true;
    }

    if (i != 1 and is_flag) {
      if (!key.empty()) {
        goto SetArgSection;  // NOLINT(cppcoreguidelines-avoid-goto)
      }
      if (!short_keys.empty()) {
        goto SetShortArgSection;  // NOLINT(cppcoreguidelines-avoid-goto)
      }
      if constexpr (!std::is_same_v<PArgs, std::tuple<>>) {
        if (!values.empty()) {
          goto SetArgSection;  // NOLINT(cppcoreguidelines-avoid-goto)
        }
      } else {
        if (!values.empty()) [[unlikely]] {
          throw InvalidArgument(
              std::format("Invalid positional argument: {}", values));
        }
      }
    SetArgSection:
      this->setArg(key, values);
      key = "";
      values.clear();
      goto End;  // NOLINT(cppcoreguidelines-avoid-goto)
    SetShortArgSection:
      this->setShortKeyArg(short_keys, values);
      short_keys = "";
      values.clear();
    End:
    }

    if (i == cmd_end_pos) {
      break;
    }

    if (is_flag) {
      if (arg.size() > 1 and arg.at(1) == '-') {
        if (arg.contains('=')) [[unlikely]] {
          auto equal_pos = arg.find('=');
          key = arg.substr(2, equal_pos - 2);
          values.push_back(arg.substr(equal_pos + 1));
          is_flag = true;
        } else {
          key = arg.substr(2);
        }
      } else {
        short_keys = arg.substr(1);
      }
    } else {
      values.push_back(arg);
    }
  }

  auto required_keys = std::vector<std::string_view>();
  tuple_type_visit<decltype(std::tuple_cat(std::declval<Args>(),
                                           std::declval<PArgs>()))>(
      [&required_keys]<class T>(T) {
        if constexpr (std::derived_from<typename T::type, ArgTag>) {
          if ((T::type::required && !T::type::assigned)) {
            required_keys.push_back(T::type::name.getKey());
          }
        }
      });

  if (!required_keys.empty()) [[unlikely]] {
    throw InvalidArgument(std::format("Requried {}", required_keys));
  }
  if (subcmd_found_idx != -1) {
    MetaParse(subParsers, subcmd_found_idx, argc - cmd_end_pos,
              &argv[cmd_end_pos]);
  }
  this->parsed_ = true;
}

struct AnsiEscapeCode {
  bool isEnabled;

  std::string bold = "\x1B[1m";
  std::string underline = "\x1B[4m";
  std::string reset = "\x1B[0m";

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
    return isEnabled ? this->getBold() + this->getUnderline() : "";
  }
};

constexpr std::size_t max_option_width = 26;

constexpr auto createUsageSection(const auto& program_name,
                                  const auto& help_info, const auto& pargs_info,
                                  const auto& sub_commands) {
  std::string ret;
  ret.append(program_name);
  for (const auto& i : help_info) {
    if (i.required) {
      ret.append(std::format(" {} {}",
                             i.shortName != '\0'
                                 ? std::format("-{}", i.shortName)
                                 : std::format("--{}", i.name),
                             i.typeName));
    }
  }
  ret.append(" [options...]");
  for (const auto& i : pargs_info) {
    if (i.required) {
      ret.append(std::format(" {}", i.name));
    } else {
      ret.append(std::format(" [{}]", i.name));
    }
  }
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

constexpr auto createSubcommandSection(const auto& ansi,
                                       const auto& sub_commands) {
  std::string ret;
  ret.push_back('\n');
  for (const auto& command : sub_commands) {
    auto description = splitStringView(command.description, '\n');
    if (command.name.size() < max_option_width - 2 and description[0] != "") {
      ret.append(std::format(
          "  {0}{1}{2}{3}{4}\n",
          ansi.getBold(),                                                // 1
          command.name,                                                  // 2
          ansi.getReset(),                                               // 3
          std::string(max_option_width - 2 - command.name.size(), ' '),  // 4
          description[0]));
    } else {
      ret.append(std::format("  {0}{1}{2}\n", ansi.getBold(), command.name,
                             ansi.getReset()));
      if (description[0] != "") {
        ret.append(std::format("{0}{1}\n", std::string(max_option_width, ' '),
                               description[0]));
      }
    }
    for (std::size_t i = 1; i < description.size(); i++) {
      ret.append(std::format("{0}{1}\n", std::string(max_option_width, ' '),
                             description[i]));
    }
  }
  return ret;
}

constexpr auto createOptionsSection(const auto& ansi, const auto& help_info) {
  std::string ret;

  ret.push_back('\n');
  for (const auto& option : help_info) {
    auto description = splitStringView(option.description, '\n');
    std::string option_string =
        std::format("{0}{1}--{2}{3} {4}",
                    ansi.getBold(),  // 0
                    (option.shortName == '\0')
                        ? "   "                                   //
                        : std::format("-{},", option.shortName),  // 1
                    option.name,                                  // 2
                    ansi.getReset(),                              // 3
                    option.typeName                               // 4
        );

    if (option_string.size() - ansi.getBold().size() - ansi.getReset().size() <
            max_option_width - 2 and
        description[0] != "") {
      ret.append(std::format(
          "  {0}{1}{2}\n",
          option_string,  // 1
          std::string(max_option_width - option_string.size() +
                          ansi.getBold().size() + ansi.getReset().size() - 2,
                      ' '),  // 3
          description[0]     // 4
          ));
    } else {
      ret.append(std::format("  {0}{1}{2}\n",
                             ansi.getBold(),  // 0
                             option_string,   // 1
                             ansi.getReset()  // 2
                             ));
      if (description[0] != "") {
        ret.append(std::format("{0}{1}\n", std::string(max_option_width, ' '),
                               description[0]));
      }
    }
    for (std::size_t i = 1; i < description.size(); i++) {
      ret.append(std::format("{0}{1}\n",                          //
                             std::string(max_option_width, ' '),  // 0
                             description[i]                       // 1
                             ));
    }
  }
  return ret;
}

constexpr auto createPositionalArgumentSection(const auto& ansi,
                                               const auto& pargs_info) {
  std::string ret;
  ret.push_back('\n');
  for (const auto& i : pargs_info) {
    auto description = splitStringView(i.description, '\n');

    if (i.name.size() < max_option_width - 2 and description[0] != "") {
      ret.append(std::format(
          "  {0}{1}{2}{3}{4}\n",
          ansi.getBold(),                                          // 1
          i.name,                                                  // 2
          ansi.getReset(),                                         // 3
          std::string(max_option_width - 2 - i.name.size(), ' '),  // 4
          description[0]));
    } else {
      ret.append(std::format("  {0}{1}{2}\n", ansi.getBold(), i.name,
                             ansi.getReset()));
      if (description[0] != "") {
        ret.append(std::format("{0}{1}\n", std::string(max_option_width, ' '),
                               description[0]));
      }
    }
    for (std::size_t i = 1; i < description.size(); i++) {
      ret.append(std::format("{0}{1}\n", std::string(max_option_width, ' '),
                             description[i]));
    }
  }
  return ret;
}

template <ParserID ID, class Args, class PArgs, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
constexpr auto Parser<ID, Args, PArgs, HArg, SubParsers>::formatHelp(
    bool no_color) const -> std::string {
  std::string ret;

  AnsiEscapeCode ansi((::isatty(1) != 0) and !no_color);

  std::vector<ArgInfo> help_info;
  if constexpr (std::is_same_v<HArg, void>) {
    help_info = HelpGenerator<Args>();
  } else {
    help_info = HelpGenerator<tuple_append_t<Args, HArg>>();
  }
  std::vector<ArgInfo> pargs_info = HelpGenerator<PArgs>();

  auto sub_commands = SubParserInfo(subParsers);

  // [[assume(this->info_)]]; // TODO(gen740): add assume when clang supports it

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
    ret.append(ansi.getBoldUnderline() + "Usage:" + ansi.getReset() + "\n");
    ret.append("  ");

    ret.append(this->info_->usage.value_or(
        createUsageSection(this->info_->program_name.value_or("no_name"),
                           help_info, pargs_info, sub_commands)));

    // Subcommand Section
    if constexpr (!std::is_same_v<SubParsers, std::tuple<>>) {
      ret.push_back('\n');
      ret.append(ansi.getBoldUnderline() + "Subcommands:" + ansi.getReset());
      ret.append(this->info_->subcommand_help.value_or(
          createSubcommandSection(ansi, sub_commands)));
    }

    if constexpr (!std::is_same_v<PArgs, std::tuple<>>) {
      ret.push_back('\n');
      ret.append(ansi.getBoldUnderline() +
                 "Positional Argument:" + ansi.getReset());
      ret.append(this->info_->positional_argument_help.value_or(
          createPositionalArgumentSection(ansi, pargs_info)));
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

