module;

export module Argo:Validation;

import :Exceptions;
import :TypeTraits;
import :std_module;

// generator start here

namespace Argo::Validation {

export struct ValidationBase {
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

export template <class T>
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

export template <std::derived_from<Argo::Validation::ValidationBase> Lhs,
                 std::derived_from<Argo::Validation::ValidationBase> Rhs>
auto operator&(Lhs lhs, Rhs rhs) {
  return Argo::Validation::AndValidation(lhs, rhs);
}

export template <std::derived_from<Argo::Validation::ValidationBase> Lhs,
                 std::derived_from<Argo::Validation::ValidationBase> Rhs>
auto operator|(Lhs lhs, Rhs rhs) {
  return Argo::Validation::OrValidation(lhs, rhs);
}

export template <std::derived_from<Argo::Validation::ValidationBase> Lhs,
                 std::derived_from<Argo::Validation::ValidationBase> Rhs>
auto operator!(Rhs rhs) {
  return Argo::Validation::InvertValidation(rhs);
}

// generator end here
