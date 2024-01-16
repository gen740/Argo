module;

// TODO(gen740): need to replace std
// Declare all stds in this file
#include <unistd.h>

#include <array>
#include <charconv>
#include <concepts>
#include <expected>
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

export module Argo:std_module;

// NOLINTBEGIN(bugprone-reserved-identifier)
export {
  using ::isatty;
  using ::std::int64_t;
  using ::std::size_t;
}

export namespace std {
using ::std::exit;

// utility
using ::std::get;
using ::std::operator==;
using ::std::operator+;
using ::std::begin;
using ::std::end;
using ::std::exchange;
using ::std::forward;
using ::std::move;

// cstdint
using ::std::int64_t;
using ::std::size_t;

// string
using ::std::stod;
using ::std::stoi;
using ::std::string;

// string_view
using ::std::string_view;

// charconv
using ::std::from_chars;

// iostream
using ::std::cout;
using ::std::endl;

// format
using ::std::format;

// array
using ::std::array;
using ::std::vector;

// span
using ::std::span;

// tuple
using ::std::apply;
using ::std::make_tuple;
using ::std::tuple;
using ::std::tuple_cat;
using ::std::tuple_element;
using ::std::tuple_element_t;
using ::std::tuple_size;
using ::std::tuple_size_v;

// type_traits
using ::std::conditional;
using ::std::conditional_t;
using ::std::declval;
using ::std::enable_if;
using ::std::enable_if_t;
using ::std::false_type;
using ::std::index_sequence;
using ::std::is_arithmetic_v;
using ::std::is_convertible;
using ::std::is_convertible_v;
using ::std::is_floating_point_v;
using ::std::is_integral_v;
using ::std::is_invocable;
using ::std::is_invocable_v;
using ::std::is_same_v;
using ::std::make_index_sequence;
using ::std::remove_cv_t;
using ::std::remove_cvref;
using ::std::remove_cvref_t;
using ::std::remove_pointer;
using ::std::remove_pointer_t;
using ::std::true_type;
using ::std::type_identity;

// concept
using ::std::derived_from;

// exception/stdexcept
using ::std::exception;
using ::std::invalid_argument;
using ::std::runtime_error;

// memory
using ::std::make_unique;
using ::std::unique_ptr;

// functional
using ::std::function;
using ::std::ref;
using ::std::reference_wrapper;

// filesystem
namespace filesystem {
using ::std::filesystem::path;
}

// optional
using ::std::nullopt;
using ::std::optional;

// expected
using ::std::expected;
using ::std::unexpected;

// NOLINTEND(bugprone-reserved-identifier)

}  // namespace std
