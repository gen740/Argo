module;

// Declare all stds in this file
#include <array>
#include <cstring>
#include <exception>
#include <format>
#include <print>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

export module std_module;

export namespace std {
// utility
using ::std::get;

// cstdint
using ::std::size_t;

// string
using ::std::stoi;
using ::std::stof;
using ::std::string;

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

// exception/stdexcept
using ::std::exception;
using ::std::invalid_argument;
using ::std::runtime_error;

}  // namespace std
