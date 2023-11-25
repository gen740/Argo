module;

import std_module;
export module Argo:Initializer;

import :Validation;
import :NArgs;
import :Arg;

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

template <class Type, auto Name, char ShortName, int ID>
struct Initializer {
  template <class Head, class... Tails>
  static auto init(Head head, Tails... tails) {
    if constexpr (std::is_same_v<Head, withDescription>) {
      Arg<Type, Name, ShortName, ID>::description = head.getDescription();
    } else if constexpr (std::derived_from<std::remove_cvref_t<std::remove_pointer_t<Head>>,
                                           Validation::ValidationTag>) {
      Arg<Type, Name, ShortName, ID>::validator = head;
    } else if constexpr (std::is_same_v<std::remove_cvref_t<Head>, NArgs>) {
      Arg<Type, Name, ShortName, ID>::nargs = head;
    } else {
      Arg<Type, Name, ShortName, ID>::defaultValue = static_cast<Type>(head);
    }
    if constexpr (sizeof...(Tails) != 0) {
      init(tails...);
    }
  }

  static auto init() {}
};

};  // namespace Argo
