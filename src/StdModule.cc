module;

// Declare all stds in this file
#include <array>
#include <cstring>
#include <exception>
#include <format>
#include <print>
#include <stdexcept>
#include <string>
#include <charconv>
#include <string_view>
#include <tuple>
#include <utility>

export module std_module;

export namespace std {
// utility
using ::std::get;
using ::std::operator==;
using ::std::begin;
using ::std::end;

// cstdint
using ::std::size_t;

// string
using ::std::stoi;
using ::std::stof;
using ::std::stod;
using ::std::string;
using ::std::string_literals::operator""s;

// string_view
using ::std::string_view;
using ::std::string_view_literals::operator""sv;

// charconv
using ::std::from_chars;
using ::std::chars_format;

// cstring
using ::std::strlen;
using ::std::strcmp;

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
using ::std::is_same_v;
using ::std::remove_cvref;
using ::std::remove_cvref_t;
using ::std::is_integral_v;
using ::std::is_floating_point_v;
using ::std::void_t;

// exception/stdexcept
using ::std::exception;
using ::std::invalid_argument;
using ::std::runtime_error;

}  // namespace std
