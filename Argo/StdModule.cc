module;

// TODO(gen740): need to replace std
// Declare all stds in this file
#include <unistd.h>

#include <array>
#include <charconv>
#include <concepts>
#include <cstring>
#include <exception>
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

#pragma clang diagnostic push
export module Argo:std_module;

export {
  using ::__stderrp;  // NOLINT(misc-unused-using-decls)
  using ::isatty;     // NOLINT(misc-unused-using-decls)
}

export namespace std {
using ::std::exit;  // NOLINT(misc-unused-using-decls)

// utility
using ::std::get;          // NOLINT(misc-unused-using-decls)
using ::std::operator==;   // NOLINT(misc-unused-using-decls)
using ::std::operator+;    // NOLINT(misc-unused-using-decls)
using ::std::begin;        // NOLINT(misc-unused-using-decls)
using ::std::end;          // NOLINT(misc-unused-using-decls)
using ::std::exchange;     // NOLINT(misc-unused-using-decls)
using ::std::forward;      // NOLINT(misc-unused-using-decls)
using ::std::move;         // NOLINT(misc-unused-using-decls)
using ::std::unreachable;  // NOLINT(misc-unused-using-decls)

// cstdint
using ::std::int64_t;  // NOLINT(misc-unused-using-decls)
using ::std::size_t;   // NOLINT(misc-unused-using-decls)

// string
using ::std::stod;                          // NOLINT(misc-unused-using-decls)
using ::std::stof;                          // NOLINT(misc-unused-using-decls)
using ::std::stoi;                          // NOLINT(misc-unused-using-decls)
using ::std::string;                        // NOLINT(misc-unused-using-decls)
using ::std::string_literals::operator""s;  // NOLINT(misc-unused-using-decls)

// string_view
using ::std::string_view;  // NOLINT(misc-unused-using-decls)
using ::std::string_view_literals::
operator""sv;  // NOLINT(misc-unused-using-decls)

// charconv
using ::std::chars_format;  // NOLINT(misc-unused-using-decls)
using ::std::from_chars;    // NOLINT(misc-unused-using-decls)

// cstring
using ::std::strcmp;  // NOLINT(misc-unused-using-decls)
using ::std::strlen;  // NOLINT(misc-unused-using-decls)

// print
using ::std::format;   // NOLINT(misc-unused-using-decls)
using ::std::print;    // NOLINT(misc-unused-using-decls)
using ::std::println;  // NOLINT(misc-unused-using-decls)

// array
using ::std::array;   // NOLINT(misc-unused-using-decls)
using ::std::vector;  // NOLINT(misc-unused-using-decls)

// span
using ::std::span;  // NOLINT(misc-unused-using-decls)

// tuple
using ::std::apply;            // NOLINT(misc-unused-using-decls)
using ::std::make_tuple;       // NOLINT(misc-unused-using-decls)
using ::std::tuple;            // NOLINT(misc-unused-using-decls)
using ::std::tuple_cat;        // NOLINT(misc-unused-using-decls)
using ::std::tuple_element;    // NOLINT(misc-unused-using-decls)
using ::std::tuple_element_t;  // NOLINT(misc-unused-using-decls)
using ::std::tuple_size;       // NOLINT(misc-unused-using-decls)
using ::std::tuple_size_v;     // NOLINT(misc-unused-using-decls)

// type_traits
using ::std::conditional;          // NOLINT(misc-unused-using-decls)
using ::std::conditional_t;        // NOLINT(misc-unused-using-decls)
using ::std::declval;              // NOLINT(misc-unused-using-decls)
using ::std::enable_if;            // NOLINT(misc-unused-using-decls)
using ::std::enable_if_t;          // NOLINT(misc-unused-using-decls)
using ::std::false_type;           // NOLINT(misc-unused-using-decls)
using ::std::index_sequence;       // NOLINT(misc-unused-using-decls)
using ::std::is_arithmetic_v;      // NOLINT(misc-unused-using-decls)
using ::std::is_convertible;       // NOLINT(misc-unused-using-decls)
using ::std::is_convertible_v;     // NOLINT(misc-unused-using-decls)
using ::std::is_floating_point_v;  // NOLINT(misc-unused-using-decls)
using ::std::is_integral_v;        // NOLINT(misc-unused-using-decls)
using ::std::is_invocable;         // NOLINT(misc-unused-using-decls)
using ::std::is_invocable_v;       // NOLINT(misc-unused-using-decls)
using ::std::is_same_v;            // NOLINT(misc-unused-using-decls)
using ::std::make_index_sequence;  // NOLINT(misc-unused-using-decls)
using ::std::remove_cv_t;          // NOLINT(misc-unused-using-decls)
using ::std::remove_cvref;         // NOLINT(misc-unused-using-decls)
using ::std::remove_cvref_t;       // NOLINT(misc-unused-using-decls)
using ::std::remove_pointer;       // NOLINT(misc-unused-using-decls)
using ::std::remove_pointer_t;     // NOLINT(misc-unused-using-decls)
using ::std::true_type;            // NOLINT(misc-unused-using-decls)
using ::std::type_identity;        // NOLINT(misc-unused-using-decls)
using ::std::type_identity_t;      // NOLINT(misc-unused-using-decls)
using ::std::void_t;               // NOLINT(misc-unused-using-decls)

// concept
using ::std::common_with;   // NOLINT(misc-unused-using-decls)
using ::std::derived_from;  // NOLINT(misc-unused-using-decls)
using ::std::same_as;       // NOLINT(misc-unused-using-decls)

// exception/stdexcept
using ::std::exception;         // NOLINT(misc-unused-using-decls)
using ::std::invalid_argument;  // NOLINT(misc-unused-using-decls)
using ::std::runtime_error;     // NOLINT(misc-unused-using-decls)

// memory
using ::std::make_unique;  // NOLINT(misc-unused-using-decls)
using ::std::unique_ptr;   // NOLINT(misc-unused-using-decls)

// functional
using ::std::function;           // NOLINT(misc-unused-using-decls)
using ::std::ref;                // NOLINT(misc-unused-using-decls)
using ::std::reference_wrapper;  // NOLINT(misc-unused-using-decls)

// optional
using ::std::nullopt;    // NOLINT(misc-unused-using-decls)
using ::std::nullopt_t;  // NOLINT(misc-unused-using-decls)
using ::std::optional;   // NOLINT(misc-unused-using-decls)

}  // namespace std