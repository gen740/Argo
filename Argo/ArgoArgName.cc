module;

export module Argo:ArgName;

import :std_module;

// generator start here

namespace Argo {

using namespace std;

/*!
 * ArgName which holds argument name
 */
template <size_t N>
struct ArgName {
  char name[N] = {};
  char shortName = '\0';
  size_t nameLen = N;

  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval ArgName(const char (&lhs)[N + 1]) {
    for (size_t i = 0; i < N; i++) {
      if (lhs[i] == ',') {
        nameLen = i;
        shortName = lhs[i + 1];
        return;
      }
      this->name[i] = lhs[i];
    }
  };

  [[nodiscard]] constexpr char operator[](size_t idx) const {
    return this->name[idx];
  }

  constexpr char& operator[](size_t idx) {
    return this->name[idx];
  }

  [[nodiscard]] constexpr auto begin() const {
    return &this->name[0];
  }

  [[nodiscard]] constexpr auto end() const {
    return &this->name[this->nameLen];
  }

  [[nodiscard]] constexpr auto size() const {
    return N;
  }

  [[nodiscard]] friend constexpr auto begin(const ArgName& lhs) {
    return lhs.begin();
  }

  [[nodiscard]] friend constexpr auto end(const ArgName& lhs) {
    return lhs.end();
  }

  template <size_t M>
  [[nodiscard]] constexpr auto operator==(const ArgName<M>& lhs) -> bool {
    if constexpr (M != N) {
      return false;
    } else {
      for (size_t i = 0; i < N; i++) {
        if ((*this)[i] != lhs[i]) {
          return false;
        }
      }
      return true;
    }
  }

  template <size_t M>
  [[nodiscard]] constexpr auto operator==(const ArgName<M>& lhs) const -> bool {
    auto NV = this->nameLen;
    auto MV = lhs.nameLen;

    if (MV != NV) {
      return false;
    }
    for (size_t i = 0; i < NV; i++) {
      if ((*this)[i] != lhs[i]) {
        return false;
      }
    }
    return true;
  }

  // NOLINTNEXTLINE(google-explicit-constructor)
  [[nodiscard]] constexpr operator string_view() const {
    return string_view(this->begin(), this->end());
  }

  [[nodiscard]] consteval auto hasValidNameLength() const -> bool {
    if (this->shortName == '\0') {
      return true;
    }
    return (N - this->nameLen) == 2;
  }
};

template <size_t N>
ArgName(const char (&)[N]) -> ArgName<N - 1>;

template <class T>
concept ArgNameType = requires(T& x) {
  static_cast<string_view>(x);
  is_same_v<decltype(x.shortName), char>;
};

}  // namespace Argo

// generator end here
