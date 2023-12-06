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

/*!
 * (default)?  : If value specified use it else use default -> ValueType
 *          int: Exactly (n > 1)                     -> std::array<ValueType, N>
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

};  // namespace Argo


namespace Argo {

class ParserInternalError : public std::runtime_error {
 public:
  explicit ParserInternalError(const std::string& msg)
      : std::runtime_error(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return std::runtime_error::what();
  }
};

class ParseError : public std::runtime_error {
 public:
  explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return std::runtime_error::what();
  }
};

/*!
 * InvalidArgument exception class
 */
class InvalidArgument : public std::invalid_argument {
 public:
  explicit InvalidArgument(const std::string& msg)
      : std::invalid_argument(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return std::invalid_argument::what();
  }
};

class ValidationError : public InvalidArgument {
 public:
  explicit ValidationError(const std::string& msg) : InvalidArgument(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
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

};  // namespace Argo


namespace Argo::Validation {

struct ValidationBase {
  template <class T>
  auto operator()(const T& value, std::span<std::string_view> values,
                  std::string_view option_name) -> void {
    if (!this->isValid(value, values)) {
      throw ValidationError(
          std::format("Option {} has invalid value {}", option_name, value));
    }
  }

  template <class T>
  auto operator()(const T& value, std::span<std::string_view> raw_val) {
    return this->isValid(value, raw_val);
  }

  template <class T>
  auto isValid(const T& /* unused */,
               std::span<std::string_view> /* unuesd */) const -> bool {
    static_assert(false, "Invalid validation");
  };

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
  auto isValid(const U& value,
               std::span<std::string_view> raw_values) const -> bool {
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
  auto isValid(const U& value,
               std::span<std::string_view> raw_values) const -> bool {
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
  InvertValidation(Rhs rhs) : rhs_(rhs){};

  template <class U>
  auto isValid(const U& value,
               std::span<std::string_view> raw_values) const -> bool {
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
    if (!this->isValid(value, values)) {
      throw ValidationError(
          std::format("Option {} has invalid value {}", option_name, value));
    }
  }

  template <class U>
  auto isValid(const U& value,
               std::span<std::string_view> /* unused */) const -> bool {
    return static_cast<U>(this->min_) < value &&
           value < static_cast<U>(this->max_);
  };
};

template <class T>
Range(T min, T max) -> Range<T>;

// template <class Type>
// struct Callback final : public ValidationBase<Type> {
//  private:
//   std::function<bool(Type)> callback_;
//
//  public:
//   Callback(std::function<bool(Type)> callback) : callback_(callback){};
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
      ret = ret + base_type_name;
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
  inline static std::string typeName = "";
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
  inline static std::string typeName = "";
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

}  // namespace Argo

namespace Argo {

constexpr auto description(std::string_view desc) -> Description {
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
    if constexpr (std::is_same_v<Head, Description>) {
      Arg::description = head.description;
    } else if constexpr (std::derived_from<std::remove_cvref_t<Head>,
                                           Validation::ValidationBase>) {
      static_assert(std::is_invocable_v<decltype(head),
                                        typename Arg::type,
                                        std::span<std::string_view>,
                                        std::string_view>);
      Arg::validator = head;
    } else if constexpr (std::derived_from<std::remove_cvref_t<Head>,
                                           ImplicitDefaultValueTag>) {
      Arg::defaultValue = static_cast<Type>(head.implicit_default_value);
    } else if constexpr (std::derived_from<std::remove_cvref_t<Head>,
                                           ExplicitDefaultValueTag>) {
      Arg::value = static_cast<Type>(head.explicit_default_value);
    } else if constexpr (std::is_invocable_v<Head,
                                             typename Arg::type&,
                                             std::span<std::string_view>>) {
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
    if constexpr (std::is_same_v<Head, Description>) {
      FlagArg::description = head.description;
    } else if constexpr (std::derived_from<std::remove_cvref_t<Head>,
                                           Validation::ValidationBase>) {
      static_assert(false, "Flag cannot have validator");
    } else if constexpr (std::derived_from<std::remove_cvref_t<Head>,
                                           ImplicitDefaultValueTag>) {
      static_assert(false, "Flag cannot have implicit default value");
    } else if constexpr (std::derived_from<std::remove_cvref_t<Head>,
                                           ExplicitDefaultValueTag>) {
      static_assert(false, "Flag cannot have explicit default value");
    } else if constexpr (std::is_invocable_v<Head>) {
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

struct ArgInfo {
  std::string_view name;
  char shortName;
  std::string_view description;
  bool required;
  std::string typeName;
};

template <class Args>
struct HelpGenerator {};

template <class... Args>
struct HelpGenerator<std::tuple<Args...>> {
  static auto generate() -> std::vector<ArgInfo> {
    std::vector<ArgInfo> ret;
    (
        [&ret]<class T>() {
          ret.emplace_back(
              std::string_view(Args::name).substr(0, Args::name.nameLen),
              Args::name.shortName,
              Args::description,
              Args::required,
              Args::typeName);
        }.template operator()<Args>(),
        ...);
    return ret;
  }
};

struct SubCommandInfo {
  std::string_view name;
  std::string_view description;
};

template <class T>
auto SubParserInfo(T subparsers) {
  std::vector<SubCommandInfo> ret{};
  if constexpr (!std::is_same_v<T, std::tuple<>>) {
    std::apply(
        [&ret]<class... Parser>(Parser... parser) {
          (..., ret.emplace_back(parser.name, parser.description));
        },
        subparsers);
  }
  return ret;
};

}  // namespace Argo


namespace Argo {

template <class Arguments>
struct GetNameFromShortName {
  template <class Head, class... Tails>
  struct GetNameFromShortNameImpl {
    [[noreturn]] static auto get(char /* unused */) -> std::string_view {
      throw ParserInternalError("Fail to lookup");
    }
  };

  template <class Head, class... Tails>
  struct GetNameFromShortNameImpl<std::tuple<Head, Tails...>> {
    static auto get(char key) -> std::string_view {
      auto name = std::string_view(Head::name);
      if (Head::name.shortName == key) {
        return name;
      }
      return GetNameFromShortNameImpl<std::tuple<Tails...>>::get(key);
    }
  };

  static auto eval(char key) -> std::string_view {
    return GetNameFromShortNameImpl<Arguments>::get(key);
  }
};

/*!
 * Index Search meta function
 */
template <class Tuple, ArgName T, int Index = 0>
struct SearchIndex;

template <ArgName T, std::size_t Index>
struct SearchIndex<std::tuple<>, T, Index> {
  static constexpr int value = -1;
};

template <ArgName T, std::size_t Index, class Head, class... Tails>
struct SearchIndex<std::tuple<Head, Tails...>, T, Index> {
  static constexpr int value =
      (Head::name == T)
          ? Index
          : SearchIndex<std::tuple<Tails...>, T, Index + 1>::value;
};

/*!
 * Index Search meta function
 */
template <class Tuple, char T, int Index = 0>
struct SearchIndexFromShortName;

template <char T, std::size_t Index>
struct SearchIndexFromShortName<std::tuple<>, T, Index> {
  static constexpr int value = -1;
};

template <char T, std::size_t Index, class Head, class... Tails>
struct SearchIndexFromShortName<std::tuple<Head, Tails...>, T, Index> {
  static constexpr int value =
      Head::name.shortName == T
          ? Index
          : SearchIndexFromShortName<std::tuple<Tails...>, T, Index + 1>::value;
};

};  // namespace Argo


namespace Argo {

/*!
 * Helper class of assigning value
 */
template <class Type>
constexpr auto caster(std::string_view value) -> Type {
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
    throw ParserInternalError("Invalid argument expect bool");
  } else if constexpr (std::is_integral_v<Type>) {
    Type tmpValue;
    std::from_chars(std::begin(value), std::end(value), tmpValue);
    return tmpValue;
  } else if constexpr (std::is_floating_point_v<Type>) {
    return static_cast<Type>(std::stod(std::string(value)));
  } else if constexpr (std::is_same_v<Type, const char*>) {
    return value.data();
  } else {
    return static_cast<Type>(value);
  }
}

template <class... T, std::size_t... N>
constexpr auto tupleAssign(std::tuple<T...>& t, std::span<std::string_view> v,
                           std::index_sequence<N...> /* unused */) {
  ((std::get<N>(t) =
        caster<std::remove_cvref_t<decltype(std::get<N>(t))>>(v[N])),
   ...);
}

template <class Arguments, class PositionalArgument>
struct Assigner {
  template <ArgType Head>
  static constexpr auto assignOneArg(
      std::string_view key, std::span<std::string_view> values) -> bool {
    if constexpr (std::derived_from<Head, FlagArgTag>) {
      if (!values.empty()) {
        if constexpr (std::is_same_v<PositionalArgument, void>) {
          throw Argo::InvalidArgument(
              std::format("Flag {} can not take value", key));
        } else {
          if (PositionalArgument::assigned) {
            throw Argo::InvalidArgument(
                std::format("Flag {} can not take value", key));
          }
          Head::value = true;
          Head::assigned = true;
          assignOneArg<PositionalArgument>(
              std::string_view(PositionalArgument::name), values);
          return true;
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
        if constexpr (std::is_same_v<PositionalArgument, void>) {
          throw Argo::InvalidArgument(
              std::format("Argument {} cannot take more than one value got {}",
                          key,
                          values.size()));
        } else {
          if (PositionalArgument::assigned) {
            throw Argo::InvalidArgument(std::format(
                "Argument {} cannot take more than one value got {}",
                key,
                values.size()));
          }
          assignOneArg<Head>(key, values.subspan(0, 1));
          assignOneArg<PositionalArgument>(
              std::string_view(PositionalArgument::name), values.subspan(1));
          return true;
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
              std::format("Argument {} should take more than one value", key));
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
          throw Argo::InvalidArgument(std::format(
              "Argument {} should take exactly one value but zero", key));
        }
        if (values.size() > 1) {
          if constexpr (std::is_same_v<PositionalArgument, void>) {
            throw Argo::InvalidArgument(
                std::format("Argument {} should take exactly one value but {}",
                            key,
                            values.size()));
          } else {
            if (PositionalArgument::assigned) {
              throw Argo::InvalidArgument(std::format(
                  "Argument {} should take exactly one value but {}",
                  key,
                  values.size()));
            }
            assignOneArg<Head>(key, values.subspan(0, 1));
            assignOneArg<PositionalArgument>(
                std::string_view(PositionalArgument::name), values.subspan(1));
            return true;
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
            tupleAssign(Head::value,
                        values,
                        std::make_index_sequence<
                            std::tuple_size_v<typename Head::type>>());
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
              std::format("Argument {} should take exactly {} value but {}",
                          key,
                          Head::nargs.getNargs(),
                          values.size()));
        } else {
          if constexpr (std::is_same_v<PositionalArgument, void>) {
            throw Argo::InvalidArgument(
                std::format("Argument {} should take exactly {} value but {}",
                            key,
                            Head::nargs.getNargs(),
                            values.size()));
          } else {
            if (PositionalArgument::assigned) {
              throw Argo::InvalidArgument(
                  std::format("Argument {} should take exactly {} value but {}",
                              key,
                              Head::nargs.getNargs(),
                              values.size()));
            }
            assignOneArg<Head>(key, values.subspan(0, Head::nargs.getNargs()));
            assignOneArg<PositionalArgument>(
                std::string_view(PositionalArgument::name),
                values.subspan(Head::nargs.getNargs()));
            return true;
          }
        }
      }
    }
    return false;
  }

  template <class Args>
  static auto assignImpl(std::string_view key,
                         std::span<std::string_view> values) {
    [&key, &values]<std::size_t... Is>(std::index_sequence<Is...> /*unused*/) {
      if (!(... ||
            (std::string_view(std::tuple_element_t<Is, Args>::name) == key and
             assignOneArg<std::tuple_element_t<Is, Args>>(key, values)))) {
        throw Argo::InvalidArgument(std::format("Invalid argument {}", key));
      }
    }(std::make_index_sequence<std::tuple_size_v<Args>>());
  }

  template <class T>
  static auto assignFlagImpl(std::string_view key) {
    [&key]<std::size_t... Is>(std::index_sequence<Is...>) {
      if (!(... || [&key]<ArgType Head>() {
            if constexpr (std::derived_from<Head, FlagArgTag>) {
              if (std::string_view(Head::name) == key) {
                Head::value = true;
                return true;
              }
              return false;
            } else {
              return false;
            }
          }.template operator()<std::tuple_element_t<Is, T>>())) {
        throw Argo::InvalidArgument("");
      }
    }(std::make_index_sequence<std::tuple_size_v<T>>());
  }

  static auto assign(std::string_view key, std::span<std::string_view> values) {
    if (key.empty()) {
      if constexpr (!std::is_same_v<PositionalArgument, void>) {
        if (PositionalArgument::assigned) {
          throw InvalidArgument(std::format("Duplicated positional argument"));
        }
        assignOneArg<PositionalArgument>(
            std::string_view(PositionalArgument::name), values);
        return;
      } else {
        throw Argo::InvalidArgument(
            std::format("Assigner: Invalid argument {}", key));
      }
    }
    assignImpl<Arguments>(key, values);
  };

  static auto assign(std::span<char> key, std::span<std::string_view> values) {
    for (std::size_t i = 0; i < key.size() - 1; i++) {
      assignFlagImpl<Arguments>(GetNameFromShortName<Arguments>::eval(key[i]));
    }
    assignImpl<Arguments>(GetNameFromShortName<Arguments>::eval(key.back()),
                          values);
  };
};

template <class Args>
auto ValueReset() {
  []<std::size_t... Is>(std::index_sequence<Is...>) {
    (..., []<ArgType T>() {
      if (T::assigned) {
        T::value = typename T::type();
        T::assigned = false;
      }
    }.template operator()<std::tuple_element_t<Is, Args>>());
  }(std::make_index_sequence<std::tuple_size_v<Args>>());
}

};  // namespace Argo


namespace Argo {

/*!
 * Checking if the given argument is required key, cycle through all the
 * tuple argument.
 */
template <class Args>
struct RequiredChecker {};

template <ArgType... Args>
struct RequiredChecker<std::tuple<Args...>> {
  static auto check() -> std::vector<std::string_view> {
    auto required_keys = std::vector<std::string_view>();
    (
        [&required_keys]<class T>() {
          if constexpr (std::derived_from<T, ArgTag>) {
            if (T::required && !T::assigned) {
              required_keys.push_back(std::string_view(T::name));
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
struct AssignChecker<std::tuple<Args...>> {
  static auto check() -> std::vector<std::string_view> {
    auto assigned_keys = std::vector<std::string_view>();
    (
        [&assigned_keys]<class T>() {
          if constexpr (std::derived_from<T, ArgTag>) {
            if (T::assigned) {
              assigned_keys.push_back(std::string_view(T::name));
            }
          }
        }.template operator()<Args>(),
        ...);
    return assigned_keys;
  };
};

}  // namespace Argo


namespace Argo {

template <ArgName Name, class Parser>
struct SubParser {
  static constexpr auto name = Name;
  std::reference_wrapper<Parser> parser;
  std::string_view description;
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
auto MetaParse(SubParsers sub_parsers, int index, int argc,
               char** argv) -> bool {
  return std::apply(
      [&](auto&&... s) {
        std::int64_t idx = -1;
        return (
            ... ||
            (idx++, idx == index && (s.parser.get().parse(argc, argv), true)));
      },
      sub_parsers);
};

template <class SubParsers>
  requires(is_tuple_v<SubParsers>)
constexpr auto ParserIndex(SubParsers sub_parsers,  //
                           std::string_view key) -> std::int64_t {
  return std::apply(
      [&](auto&&... s) {
        std::int64_t index = -1;
        bool found = (... || (index++, s.name == key));
        return found ? index : -1;
      },
      sub_parsers);
};

}  // namespace Argo


namespace Argo {

struct Unspecified {};

enum class RequiredFlag : bool {
  optional = false,
  required = true,
};

using RequiredFlag::required;  // NOLINT(misc-unused-using-decls)
using RequiredFlag::optional;  // NOLINT(misc-unused-using-decls)

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

template <ParserID ID = 0, class Args = std::tuple<>, class PArg = void,
                 class HArg = void, class SubParsers = std::tuple<>>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
class Parser {
 private:
  bool parsed_ = false;
  std::unique_ptr<ParserInfo> info_ = nullptr;

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

  Parser(const Parser&) = delete;
  Parser(Parser&&) = delete;

  SubParsers subParsers;

  constexpr explicit Parser(SubParsers tuple) : subParsers(tuple) {}

  constexpr explicit Parser(std::unique_ptr<ParserInfo> info, SubParsers tuple)
      : info_(std::move(info)), subParsers(tuple){};

  template <class Type, ArgName Name, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), class... T>
  auto createArg(T... args) {
    static_assert(!Name.containsInvalidChar(), "Name has invalid char");
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
          return NArgs(static_cast<int>(array_len_v<Type>));
        }
        if constexpr (is_vector_v<Type>) {
          return NArgs('*');
        }
        if constexpr (is_tuple_v<Type>) {
          return NArgs(static_cast<int>(std::tuple_size_v<Type>));
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
    if constexpr (!std::is_same_v<PArg, void>) {
      static_assert(!(std::string_view(Name) == std::string_view(PArg::name)),
                    "Duplicated name");
    }
    static_assert(
        (Name.shortName == '\0') ||
            (SearchIndexFromShortName<Args, Name.shortName>::value == -1),
        "Duplicated short name");
    static_assert(                                   //
        Argo::SearchIndex<Args, Name>::value == -1,  //
        "Duplicated name");
    static_assert(                     //
        (nargs.nargs > 0               //
         || nargs.nargs_char == '?'    //
         || nargs.nargs_char == '+'    //
         || nargs.nargs_char == '*'),  //
        "nargs must be '?', '+', '*' or int");

    ArgInitializer<Type, Name, nargs, required, ID>::init(
        std::forward<T>(args)...);
    return std::type_identity<Arg<Type, Name, nargs, required, ID>>();
  }

  /*!
   * Type: type of argument
   * Name: name of argument
   * arg1: ShortName or NArgs or Unspecified
   * arg2: NArgs or Unspecified
   */
  template <ArgName Name, class Type, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), class... T>
  auto addArg(T... args) {
    auto arg = createArg<Type, Name, arg1, arg2>(std::forward<T>(args)...);
    return Parser<ID,
                  tuple_append_t<Args, typename decltype(arg)::type>,
                  PArg,
                  HArg,
                  SubParsers>(std::move(this->info_), subParsers);
  }

  template <ArgName Name, class Type, auto arg1 = Unspecified(),
            auto arg2 = Unspecified(), class... T>
  auto addPositionalArg(T... args) {
    static_assert(std::is_same_v<PArg, void>,
                  "Positional argument cannot set more than one");
    static_assert(Name.shortName == '\0',
                  "Positional argment cannot have short name");
    auto arg = createArg<Type, Name, arg1, arg2>(std::forward<T>(args)...);
    return Parser<ID, Args, typename decltype(arg)::type, HArg, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name, class... T>
  auto addFlag(T... args) {
    if constexpr (!std::is_same_v<PArg, void>) {
      static_assert(!(std::string_view(Name) == std::string_view(PArg::name)),
                    "Duplicated name");
    }
    static_assert(
        (Name.shortName == '\0') ||
            (SearchIndexFromShortName<Args, Name.shortName>::value == -1),
        "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>::value == -1,
                  "Duplicated name");
    FlagArgInitializer<Name, ID>::init(std::forward<T>(args)...);
    return Parser<ID,
                  tuple_append_t<Args, FlagArg<Name, ID>>,
                  PArg,
                  HArg,
                  SubParsers>(std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  auto addHelp() {
    static_assert((SearchIndexFromShortName<Args, Name.shortName>::value == -1),
                  "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>::value == -1,
                  "Duplicated name");
    return Parser<ID, Args, PArg, HelpArg<Name, ID>, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name = "help,h">
  auto addHelp(std::string_view help) {
    static_assert((SearchIndexFromShortName<Args, Name.shortName>::value == -1),
                  "Duplicated short name");
    static_assert(Argo::SearchIndex<Args, Name>::value == -1,
                  "Duplicated name");
    static_assert(!Name.containsInvalidChar(), "Name has invalid char");
    static_assert(Name.hasValidNameLength(),
                  "Short name can't be more than one charactor");
    this->info_->help = help;
    return Parser<ID, Args, PArg, HelpArg<Name, ID>, SubParsers>(
        std::move(this->info_), subParsers);
  }

  template <ArgName Name>
  auto getArg() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!std::is_same_v<PArg, void>) {
      if constexpr (std::string_view(Name) == std::string_view(PArg::name)) {
        return PArg::value;
      } else {
        static_assert(SearchIndex<Args, Name>::value != -1,
                      "Argument does not exist");
        return std::remove_cvref_t<
            decltype(std::get<SearchIndex<Args, Name>::value>(
                std::declval<Args>()))>::value;
      }
    } else {
      static_assert(SearchIndex<Args, Name>::value != -1,
                    "Argument does not exist");
      return std::remove_cvref_t<
          decltype(std::get<SearchIndex<Args, Name>::value>(
              std::declval<Args>()))>::value;
    }
  }

  template <ArgName Name>
  auto& getParser() {
    if constexpr (std::is_same_v<SubParsers, std::tuple<>>) {
      static_assert(false, "Parser has no sub parser");
    }
    static_assert(!(SearchIndex<SubParsers, Name>::value == -1),
                  "Could not find subparser");
    return std::get<SearchIndex<SubParsers, Name>::value>(subParsers)
        .parser.get();
  }

  template <ArgName Name>
  auto isAssigned() {
    if (!this->parsed_) {
      throw ParseError("Parser did not parse argument, call parse first");
    }
    if constexpr (!std::is_same_v<PArg, void>) {
      if constexpr (std::string_view(Name) == std::string_view(PArg::name)) {
        return PArg::assigned;
      } else {
        return std::remove_cvref_t<
            decltype(std::get<SearchIndex<Args, Name>::value>(
                std::declval<Args>()))>::assigned;
      }
    } else {
      return std::remove_cvref_t<
          decltype(std::get<SearchIndex<Args, Name>::value>(
              std::declval<Args>()))>::assigned;
    }
  }

  /*!
   * Add subcommand
   */
  template <ArgName Name, class T>
  auto addParser(T& sub_parser, Description description = {""}) {
    auto s = std::make_tuple(
        SubParser<Name, T>{std::ref(sub_parser), description.description});
    auto sub_parsers = std::tuple_cat(subParsers, s);
    return Parser<ID, Args, PArg, HArg, decltype(sub_parsers)>(
        std::move(this->info_), sub_parsers);
  }

  auto resetArgs() -> void;

  auto addUsageHelp(std::string_view usage) {
    this->info_->usage = usage;
  }

  auto addSubcommandHelp(std::string_view subcommand_help) {
    this->info_->subcommand_help = subcommand_help;
  }

  auto addPositionalArgumentHelp(std::string_view positional_argument_help) {
    this->info_->positional_argument_help = positional_argument_help;
  }

  auto addOptionsHelp(std::string_view options_help) {
    this->info_->options_help = options_help;
  }

 private:
  auto setArg(std::string_view key,
              std::span<std::string_view> val) const -> void;
  auto setArg(std::span<char> key,
              std::span<std::string_view> val) const -> void;

 public:
  auto parse(int argc, char* argv[]) -> void;
  std::string formatHelp(bool no_color = false) const;

  operator bool() const {
    return this->parsed_;
  }
};

}  // namespace Argo


namespace Argo {

inline auto splitStringView(std::string_view str,
                            char delimeter) -> std::vector<std::string_view> {
  std::vector<std::string_view> ret;
  while (str.contains(delimeter)) {
    auto pos = str.find(delimeter);
    ret.push_back(str.substr(0, pos));
    str = str.substr(pos + 1);
  }
  ret.push_back(str);
  return ret;
}

template <ParserID ID, class Args, class PArg, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArg, HArg, SubParsers>::resetArgs() -> void {
  ValueReset<Args>();
}

template <ParserID ID, class Args, class PArg, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArg, HArg, SubParsers>::setArg(
    std::string_view key, std::span<std::string_view> val) const -> void {
  if constexpr (!std::is_same_v<HArg, void>) {
    if (key == HArg::name) {
      std::println("{}", formatHelp());
      std::exit(0);
    }
  }
  Assigner<Args, PArg>::assign(key, val);
}

template <ParserID ID, class Args, class PArg, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArg, HArg, SubParsers>::setArg(
    std::span<char> key, std::span<std::string_view> val) const -> void {
  if constexpr (!std::is_same_v<HArg, void>) {
    for (const auto& i : key) {
      if constexpr (HArg::name.shortName != '\0') {
        if (i == HArg::name.shortName) {
          std::println("{}", formatHelp());
          std::exit(0);
        }
      }
    }
  }
  Assigner<Args, PArg>::assign(key, val);
}

template <ParserID ID, class Args, class PArg, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArg, HArg, SubParsers>::parse(int argc,
                                                     char* argv[]) -> void {
  if (this->parsed_) [[unlikely]] {
    throw ParseError("Cannot parse twice");
  }
  auto assigned_keys = AssignChecker<Args>::check();
  if (!assigned_keys.empty()) [[unlikely]] {
    throw ParseError(std::format("keys {} already assigned", assigned_keys));
  }

  std::int64_t subcmd_found_idx = -1;
  std::int64_t cmd_end_pos = argc;
  if constexpr (!std::is_same_v<SubParsers, std::tuple<>>) {
    for (int i = argc - 1; i > 0; i--) {
      subcmd_found_idx = ParserIndex(subParsers, argv[i]);
      if (subcmd_found_idx != -1) {
        cmd_end_pos = i;
        break;
      }
    }
  }

  std::string_view key{};
  std::vector<char> short_keys{};
  short_keys.reserve(10);
  std::vector<std::string_view> values{};
  values.reserve(10);

  assert(this->info_);  // this->info_ cannot be nullptr
  if (!this->info_->program_name) {
    this->info_->program_name = std::string_view(argv[0]);
  }

  for (int i = 1; i < cmd_end_pos; i++) {
    std::string_view arg = argv[i];
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
          if constexpr (!std::is_same_v<PArg, void>) {
            this->setArg(key, values);
            values.clear();
          }
        }
        if (arg.contains('=')) {
          auto equal_pos = arg.find('=');
          auto value = std::vector<std::string_view>{arg.substr(equal_pos + 1)};
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
        if constexpr (!std::is_same_v<PArg, void>) {
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
      if constexpr (std::is_same_v<PArg, void>) {
        if (key.empty() && short_keys.empty()) {
          throw InvalidArgument(std::format("No keys specified"));
        }
      }
      values.push_back(arg);

      if (i == cmd_end_pos - 1) {
        if (!key.empty()) {
          this->setArg(key, values);
        } else if (!short_keys.empty()) {
          this->setArg(short_keys, values);
        } else {
          if constexpr (std::is_same_v<PArg, void>) {
            throw InvalidArgument(std::format("No keys specified"));
          } else {
            this->setArg(key, values);
          }
        }
      }
    }
  }
  auto required_keys = RequiredChecker<Args>::check();
  if (!required_keys.empty()) {
    throw InvalidArgument(std::format("Requried {}", required_keys));
  }
  if (subcmd_found_idx != -1) {
    MetaParse(
        subParsers, subcmd_found_idx, argc - cmd_end_pos, &argv[cmd_end_pos]);
  }
  this->parsed_ = true;
}

struct AnsiEscapeCode {
  bool isEnabled;

  std::string bold = "\x1B[1m";
  std::string underline = "\x1B[4m";
  std::string reset = "\x1B[0m";

  auto getBold() const {
    return isEnabled ? bold : "";
  }

  auto getUnderline() const {
    return isEnabled ? underline : "";
  }

  auto getReset() const {
    return isEnabled ? reset : "";
  }

  auto getBoldUnderline() const {
    return isEnabled ? bold + underline : "";
  }
};

inline auto createUsageSection(const auto& program_name,
                               [[maybe_unused]] const auto& help_info,
                               const auto& sub_commands) {
  std::string ret;
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
  std::string ret;
  std::size_t max_command_length = 0;
  for (const auto& command : sub_commands) {
    if (max_command_length < command.name.size()) {
      max_command_length = command.name.size();
    }
  }
  for (const auto& command : sub_commands) {
    ret.push_back('\n');
    auto description = splitStringView(command.description, '\n');

    ret.append(
        std::format("  {}{} {}{} {}",
                    ansi.getBold(),
                    command.name,
                    std::string(max_command_length - command.name.size(), ' '),
                    ansi.getReset(),
                    description[0]));
    for (std::size_t i = 1; i < description.size(); i++) {
      ret.push_back('\n');
      ret.append(std::format("    {}{}",  //
                             std::string(max_command_length, ' '),
                             description[i]));
    }

    // erase trailing spaces
    auto pos = ret.find_last_not_of(' ');
    ret = ret.substr(0, pos + 1);
  }

  ret.push_back('\n');
  return ret;
}

inline auto createOptionsSection(const auto& ansi, const auto& help_info) {
  std::string ret;
  std::size_t max_name_len = 0;
  for (const auto& option : help_info) {
    if (max_name_len < option.name.size() + option.typeName.size()) {
      max_name_len = option.name.size() + option.typeName.size();
    }
  }
  for (const auto& option : help_info) {
    ret.push_back('\n');
    auto description = splitStringView(option.description, '\n');

    ret.append(std::format(
        "  {}{} --{} {}{}{}  {}",
        (option.shortName == '\0') ? "   "
                                   : std::format("-{},", option.shortName),
        ansi.getBold(),
        option.name,
        ansi.getReset(),
        option.typeName,
        std::string(max_name_len - option.name.size() - option.typeName.size(),
                    ' '),
        description[0]));
    for (std::size_t i = 1; i < description.size(); i++) {
      ret.push_back('\n');
      ret.append(std::format(
          "      {}     {}", std::string(max_name_len, ' '), description[i]));
    }

    auto pos = ret.find_last_not_of(' ');
    ret = ret.substr(0, pos + 1);
  }
  return ret;
}

template <ArgType PArg>
auto createPositionalArgumentSection(const auto& ansi) {
  std::string ret;

  ret.push_back('\n');
  auto desc = splitStringView(PArg::description, '\n');
  ret.append(std::format("  {}{}{}  {}",
                         ansi.getBold(),
                         std::string_view(PArg::name),
                         ansi.getReset(),
                         desc[0]));

  for (std::size_t i = 1; i < desc.size(); i++) {
    ret.push_back('\n');
    ret.append(std::format("{}{}", std::string(8, ' '), desc[i]));
  }

  ret.push_back('\n');
  return ret;
}

template <ParserID ID, class Args, class PArg, class HArg, class SubParsers>
  requires(is_tuple_v<Args> && is_tuple_v<SubParsers>)
auto Parser<ID, Args, PArg, HArg, SubParsers>::formatHelp(bool no_color) const
    -> std::string {
  std::string ret;

  AnsiEscapeCode ansi(::isatty(1) and !no_color);

  assert(this->info_);  // this->info_ cannot be nullptr

  auto help_info = HelpGenerator<tuple_append_t<Args, HArg>>::generate();
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
                           help_info,
                           sub_commands)));

    // Subcommand Section
    if constexpr (!std::is_same_v<SubParsers, std::tuple<>>) {
      ret.push_back('\n');
      ret.append(ansi.getBoldUnderline() + "Subcommands:" + ansi.getReset());
      ret.append(this->info_->subcommand_help.value_or(
          createSubcommandSection(ansi, sub_commands)));
    }

    if constexpr (!std::is_same_v<PArg, void>) {
      ret.push_back('\n');
      ret.append(ansi.getBoldUnderline() +
                 "Positional Argument:" + ansi.getReset());
      ret.append(this->info_->positional_argument_help.value_or(
          createPositionalArgumentSection<PArg>(ansi)));
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

