module;

export module Argo:ArgName;

import :std_module;

#include "Argo/ArgoMacros.hh"

// generator start here

namespace Argo {

using namespace std;

/*!
 * ArgName which holds argument name
 */
template <size_t N>
struct ArgName {
  char short_key_ = '\0';
  char key_[N] = {};
  size_t key_len_ = N;

  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval ArgName(const char (&lhs)[N + 1]) {
    for (size_t i = 0; i < N; i++) {
      if (lhs[i] == ',') {
        this->key_len_ = i;
        this->short_key_ = lhs[i + 1];
        return;
      }
      this->key_[i] = lhs[i];
    }
  };

  [[nodiscard]] ARGO_ALWAYS_INLINE constexpr char getShortName() const {
    return this->short_key_;
  }

  [[nodiscard]] ARGO_ALWAYS_INLINE constexpr auto getKey() const {
    return string_view(this->key_, this->key_len_);
  }

  [[nodiscard]] constexpr auto getKeyLen() const {
    return this->key_len_;
  }

  template <size_t M>
  [[nodiscard]] ARGO_ALWAYS_INLINE constexpr auto operator==(
      const ArgName<M>& lhs) const -> bool {
    return this->getKey() == lhs.getKey();
  }

  [[nodiscard]] ARGO_ALWAYS_INLINE consteval auto hasValidNameLength() const
      -> bool {
    if (this->getShortName() == '\0') {
      return true;
    }
    return (N - this->key_len_) == 2;
  }
};

template <size_t N>
ArgName(const char (&)[N]) -> ArgName<N - 1>;

template <class T>
concept ArgNameType = requires(T& x) {
  is_same_v<decltype(x.getKey()), string_view>;
  is_same_v<decltype(x.getShortName()), char>;
};

}  // namespace Argo

// generator end here
