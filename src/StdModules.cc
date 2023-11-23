module;

// Declare all stds in this file
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <ranges>
#include <shared_mutex>
#include <streambuf>
#include <thread>
#include <vector>

export module std_modules;

export namespace std {

// basic type
using ::std::int16_t;
using ::std::int32_t;
using ::std::int64_t;
using ::std::int8_t;
using ::std::int_fast16_t;
using ::std::int_fast32_t;
using ::std::int_fast64_t;
using ::std::int_fast8_t;
using ::std::int_least16_t;
using ::std::int_least32_t;
using ::std::int_least64_t;
using ::std::int_least8_t;

using ::std::uint16_t;
using ::std::uint32_t;
using ::std::uint64_t;
using ::std::uint8_t;
using ::std::uint_fast16_t;
using ::std::uint_fast32_t;
using ::std::uint_fast64_t;
using ::std::uint_fast8_t;
using ::std::uint_least16_t;
using ::std::uint_least32_t;
using ::std::uint_least64_t;
using ::std::uint_least8_t;

// list all of cmath funcion
using ::std::abs;
using ::std::acos;
using ::std::acosh;
using ::std::asin;
using ::std::asinh;
using ::std::atan;
using ::std::atan2;
using ::std::atanh;
using ::std::cbrt;
using ::std::ceil;
using ::std::copysign;
using ::std::cos;
using ::std::cosh;
using ::std::erf;
using ::std::erfc;
using ::std::exp;
using ::std::exp2;
using ::std::expm1;
using ::std::fabs;
using ::std::fdim;
using ::std::floor;
using ::std::fma;
using ::std::fmax;
using ::std::fmin;
using ::std::fmod;
using ::std::fpclassify;
using ::std::frexp;
using ::std::hypot;
using ::std::ilogb;
using ::std::isfinite;
using ::std::isgreater;
using ::std::isgreaterequal;
using ::std::isinf;
using ::std::isless;
using ::std::islessequal;
using ::std::islessgreater;
using ::std::isnan;
using ::std::isnormal;
using ::std::isunordered;
using ::std::ldexp;
using ::std::lgamma;
using ::std::llrint;
using ::std::llround;
using ::std::log;
using ::std::log10;
using ::std::log1p;
using ::std::log2;
using ::std::logb;
using ::std::lrint;
using ::std::lround;
using ::std::modf;
using ::std::nan;
using ::std::nanf;
using ::std::nanl;
using ::std::nearbyint;
using ::std::nextafter;
using ::std::nexttoward;
using ::std::pow;
using ::std::remainder;
using ::std::remquo;
using ::std::rint;
using ::std::round;
using ::std::scalbln;
using ::std::scalbn;
using ::std::signbit;
using ::std::sin;
using ::std::sinh;
using ::std::sqrt;
using ::std::tan;

// cstring
using ::std::memset;

// string
using ::std::string;

// thread
namespace this_thread {
using ::std::this_thread::sleep_for;
using ::std::this_thread::sleep_until;
} // namespace this_thread

using ::std::thread;

// iostream
using ::std::cerr;
using ::std::cout;
using ::std::endl;
using ::std::flush;
using ::std::ostream;

// fstearm
using ::std::fstream;
using ::std::ifstream;
using ::std::ios;
using ::std::ofstream;
using ::std::operator<<;

using ::std::streambuf;

// string stream
using ::std::stringstream;

// iomanip
using ::std::defaultfloat;
using ::std::fixed;
using ::std::setprecision;

// memory
using ::std::make_unique;
using ::std::unique_ptr;

// array
using ::std::array;

// vector
using ::std::vector;

// chrono
namespace chrono {
using ::std::chrono::steady_clock;
}

// atomic
using ::std::atomic_bool;
using ::std::atomic_int;
using ::std::atomic_int16_t;
using ::std::atomic_int32_t;
using ::std::atomic_int64_t;
using ::std::atomic_int8_t;
using ::std::atomic_uint16_t;
using ::std::atomic_uint32_t;
using ::std::atomic_uint64_t;
using ::std::atomic_uint8_t;

// filesystem
namespace filesystem {

using ::std::filesystem::begin;
using ::std::filesystem::directory_iterator;
using ::std::filesystem::end;
using ::std::filesystem::path;

} // namespace filesystem

// mutex
using ::std::lock_guard;
using ::std::mutex;

// shared mutex
using ::std::shared_lock;
using ::std::shared_mutex;
using ::std::unique_lock;

// ranges
namespace ranges {

using ::std::ranges::sort;

namespace views {

using ::std::ranges::views::iota;
using ::std::ranges::views::take;
using ::std::ranges::views::take_while;
using ::std::ranges::views::zip;

} // namespace views

} // namespace ranges

using ::std::begin;
using ::std::end;
using ::std::less;

namespace views = ranges::views;

} // namespace std