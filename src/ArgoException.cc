module;

import std_module;
export module Argo:Exception;

export namespace Argo {

/*!
 * InvalidArgument exception class
 */
class InvalidArgument : public std::invalid_argument {
 public:
  explicit InvalidArgument(const std::string& msg) : std::invalid_argument(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return std::invalid_argument::what();
  }
};

class ParseError : public std::runtime_error {
 public:
  explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return std::runtime_error::what();
  }
};

class ParserInternalError : public std::runtime_error {
 public:
  explicit ParserInternalError(const std::string& msg) : std::runtime_error(msg) {}

  [[nodiscard]] const char* what() const noexcept override {
    return std::runtime_error::what();
  }
};

}  // namespace Argo
