export module Argo:Exceptions;

import :std_module;

// generator start here

namespace Argo {

export class ParserInternalError : public std::runtime_error {
 public:
  explicit ParserInternalError(const std::string& msg) : runtime_error(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return runtime_error::what();
  }
};

export class ParseError : public std::runtime_error {
 public:
  explicit ParseError(const std::string& msg) : runtime_error(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return runtime_error::what();
  }
};

/*!
 * InvalidArgument exception class
 */
export class InvalidArgument : public std::invalid_argument {
 public:
  explicit InvalidArgument(const std::string& msg) : invalid_argument(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return invalid_argument::what();
  }
};

export class ValidationError : public InvalidArgument {
 public:
  explicit ValidationError(const std::string& msg) : InvalidArgument(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return InvalidArgument::what();
  }
};

}  // namespace Argo

// generator end here
