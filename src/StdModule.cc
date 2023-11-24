module;

// Declare all stds in this file
#include <array>
#include <format>
#include <print>
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
using ::std::string;

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
using ::std::remove_cvref;
using ::std::remove_cvref_t;
using ::std::is_same_v;

}  // namespace std
