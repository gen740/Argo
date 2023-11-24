module;

import std_module;
export module Argo:Initializer;

import :Validation;

export namespace Argo {

constexpr char NULLCHAR = '\0';

struct with_description {
 private:
  std::string_view description_;

 public:
  explicit with_description(std::string_view description) : description_(description) {}

  auto getDescription() {
    return this->description_;
  }
};

/*!
 * Arg type this holds argument value
 */
template <class Type, auto Name, char ShortName = NULLCHAR>
struct Arg {
  static constexpr auto name = Name;
  static constexpr char shortName = ShortName;
  using type = Type;
  inline static type value = {};
  inline static std::string_view description;
  inline static bool flagArg = {};
  inline static Validation::ValidationBase<Type>* validator = nullptr;
};

template <class Type, auto Name, char ShortName>
struct Initializer {
  template <class Head, class... Tails>
  static auto init(Head head, Tails... tails) {
    if constexpr (std::is_same_v<Head, with_description>) {
      Arg<Type, Name, ShortName>::description = head.getDescription();
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           Validation::ValidationTag>) {
      Arg<Type, Name, ShortName>::validator = head;
    } else {
      Arg<Type, Name, ShortName>::value = static_cast<Type>(head);
    }
    if constexpr (sizeof...(Tails) != 0) {
      init(tails...);
    }
  }

  static auto init() {}
};

};  // namespace Argo
