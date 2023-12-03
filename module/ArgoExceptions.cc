module;

export module Argo:Exceptions;

import :std_module;

export namespace Argo {

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
