module;

import std_module;
export module Argo:Initializer;

import :Validation;

export namespace Argo {

constexpr char NULLCHAR = '\0';

struct withDescription {
 private:
  std::string_view description_;

 public:
  explicit withDescription(std::string_view description) : description_(description) {}

  auto getDescription() {
    return this->description_;
  }
};

/*!
 * (default)?  : If value specified use it else use default
 *          int: Exactly n value                    -> vector
 *          *  : Any number of argument             -> vector
 *          +  : Any number of argument except zero -> vector
 */
struct NArgs {
  int nargs = -1;
  char nargs_char = '?';

  template <int T>
    requires(T > 0)
  constexpr explicit NArgs() : nargs(T) {}

  template <char T>
    requires(T == '?' or T == '+' or T == '*')
  constexpr explicit NArgs() : nargs(T) {}

  constexpr explicit NArgs(int narg) : nargs(narg) {
    if (narg < 0) {
      throw std::invalid_argument(std::format("Nargs should be larger than 0, got {}", narg));
    }
  }

  constexpr explicit NArgs(char narg) : nargs_char(narg) {
    if (narg != '?' && narg != '*' && narg != '+') {
      throw std::invalid_argument(std::format("Nargs should be '?', '*' or '+', got {}", narg));
    }
  }

 private:
  template <class Type, auto Name, char ShortName, int ID>
  friend struct Arg;
};

/*!
 * Arg type this holds argument value
 */
template <class Type, auto Name, char ShortName, int ID>
struct Arg {
  static constexpr auto name = Name;
  static constexpr char shortName = ShortName;
  using type = Type;
  inline static std::optional<type> value = {};
  inline static std::optional<type> defaultValue = {};
  inline static std::string_view description;
  inline static bool flagArg = {};
  inline static NArgs nargs = NArgs('?');
  inline static Validation::ValidationBase<Type>* validator = nullptr;
  inline static std::function<Type(std::string_view)> caster = nullptr;
};

template <auto Name, char ShortName, int ID>
struct FlagArg {
  static constexpr auto name = Name;
  static constexpr char shortName = ShortName;
  using type = bool;
  inline static type value = false;
  inline static std::string_view description;
};

template <class Type, auto Name, char ShortName, int ID>
struct Initializer {
  template <class Head, class... Tails>
  static auto init(Head head, Tails... tails) {
    if constexpr (std::is_same_v<Head, withDescription>) {
      Arg<Type, Name, ShortName, ID>::description = head.getDescription();
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           Validation::ValidationTag>) {
      Arg<Type, Name, ShortName, ID>::validator = head;
    } else {
      // Arg<Type, Name, ShortName, ID>::value = static_cast<Type>(head);
      Arg<Type, Name, ShortName, ID>::defaultValue = static_cast<Type>(head);
    }
    if constexpr (sizeof...(Tails) != 0) {
      init(tails...);
    }
  }

  static auto init() {}
};

};  // namespace Argo
