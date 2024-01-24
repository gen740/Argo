export module Argo:ArgName;

import :std_module;

#include "Argo/ArgoMacros.hh"

// generator start here

namespace Argo {

/*!
 * ArgName which holds argument name
 */
template <std::size_t N>
struct ArgName {
  char short_key_ = '\0';
  char key_[N] = {};
  std::size_t key_len_ = N;

  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval ArgName(const char (&lhs)[N + 1]) {
    for (std::size_t i = 0; i < N; i++) {
      if (lhs[i] == ',') {
        this->key_len_ = i;
        this->short_key_ = lhs[i + 1];
        return;
      }
      this->key_[i] = lhs[i];
    }
  };

  [[nodiscard]] ARGO_ALWAYS_INLINE constexpr auto getShortName() const {
    return this->short_key_;
  }

  [[nodiscard]] ARGO_ALWAYS_INLINE constexpr auto getKey() const {
    return std::string_view(this->key_, this->key_len_);
  }

  [[nodiscard]] constexpr auto getKeyLen() const {
    return this->key_len_;
  }

  template <std::size_t M>
  [[nodiscard]] ARGO_ALWAYS_INLINE consteval auto operator==(
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

template <std::size_t N>
ArgName(const char (&)[N]) -> ArgName<N - 1>;

template <class T>
concept ArgNameType = requires(T& x) {
  std::is_same_v<decltype(x.getKey()), std::string_view>;
  std::is_same_v<decltype(x.getShortName()), char>;
};

}  // namespace Argo

// generator end here
