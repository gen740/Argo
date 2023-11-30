module;

export module Argo:Validation;

import :Exceptions;
import :TypeTraits;
import :std_module;

export namespace Argo::Validation {

struct ValidationTag {};

template <class Type>
struct ValidationBase : public ValidationTag {
  virtual auto operator()(std::string_view optionName, const Type& value) -> void {
    if (!this->isValid(value)) {
      throw ValidationError(std::format("Option {} Invalid value {}", optionName, value));
    }
  }

  virtual auto isValid(const Type& value) const -> bool = 0;
  virtual ~ValidationBase() = default;
};

template <Arithmetic Type>
struct MinMax final : public ValidationBase<Type> {
 private:
  Type min_;
  Type max_;

 public:
  MinMax(Type min, Type max) : min_(min), max_(max){};

  auto operator()(std::string_view optionName, const Type& value) -> void {
    if (!this->isValid(value)) {
      throw ValidationError(std::format("Option {}: Value must in range ({}, {}) got {}",
                                        optionName, this->min_, this->max_, value));
    }
  }

  auto isValid(const Type& value) const -> bool {
    return this->min_ < value && value < this->max_;
  };
};

}  // namespace Argo::Validation
