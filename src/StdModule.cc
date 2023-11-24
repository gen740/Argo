module;

// Declare all stds in this file
#include <array>
#include <charconv>
#include <cstring>
#include <exception>
#include <optional>
#include <format>
#include <functional>
#include <memory>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

export module std_module;

export {
  using ::__stderrp;
}

export namespace std {
// utility
using ::std::get;
using ::std::operator==;
using ::std::begin;
using ::std::end;
using ::std::forward;

// cstdint
using ::std::size_t;

// string
using ::std::stod;
using ::std::stof;
using ::std::stoi;
using ::std::string;
using ::std::string_literals::operator""s;

// string_view
using ::std::string_view;
using ::std::string_view_literals::operator""sv;

// charconv
using ::std::chars_format;
using ::std::from_chars;

// cstring
using ::std::strcmp;
using ::std::strlen;

// print
using ::std::format;
using ::std::print;
using ::std::println;

// array
using ::std::array;

// tuple
using ::std::make_tuple;
using ::std::tuple;
using ::std::tuple_cat;

// type_traits
using ::std::declval;
using ::std::is_arithmetic_v;
using ::std::is_floating_point_v;
using ::std::is_integral_v;
using ::std::is_same_v;
using ::std::remove_cvref;
using ::std::remove_cvref_t;
using ::std::remove_pointer;
using ::std::remove_pointer_t;
using ::std::void_t;

// concept
using ::std::derived_from;


// exception/stdexcept
using ::std::exception;
using ::std::invalid_argument;
using ::std::runtime_error;

// memory
using ::std::unique_ptr;

// functional
using ::std::function;

// optional
using ::std::optional;
using ::std::nullopt;
using ::std::nullopt_t;

}  // namespace std
