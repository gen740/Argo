module;

export module Argo:ArgName;

import :std_module;

// generator start here

namespace Argo {

export struct ArgNameTag {};

/*!
 * ArgName which holds argument name
 */
export template <std::size_t N>
struct ArgName : ArgNameTag {
  char name[N] = {};
  char shortName = '\0';
  std::size_t nameLen = N;

  explicit ArgName() = default;

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr ArgName(const char (&lhs)[N + 1]) {
    for (std::size_t i = 0; i < N; i++) {
      if (lhs[i] == ',') {
        nameLen = i;
        shortName = lhs[i + 1];
        return;
      }
      this->name[i] = lhs[i];
    }
  };

  constexpr char operator[](std::size_t idx) const {
    return this->name[idx];
  }

  constexpr char& operator[](std::size_t idx) {
    return this->name[idx];
  }

  constexpr auto begin() const {
    return &this->name[0];
  }

  constexpr auto end() const {
    return &this->name[this->nameLen];
  }

  constexpr auto size() const {
    return N;
  }

  friend auto begin(ArgName lhs) {
    return lhs.begin();
  }

  friend auto end(ArgName lhs) {
    return lhs.end();
  }

  template <std::size_t M>
  constexpr auto operator==(ArgName<M> lhs) -> bool {
    if constexpr (M != N) {
      return false;
    } else {
      for (std::size_t i = 0; i < N; i++) {
        if ((*this)[i] != lhs[i]) {
          return false;
        }
      }
      return true;
    }
  }

  template <std::size_t M>
  constexpr auto operator==(ArgName<M> lhs) const -> bool {
    auto NV = this->nameLen;
    auto MV = lhs.nameLen;

    if (MV != NV) {
      return false;
    }
    for (std::size_t i = 0; i < NV; i++) {
      if ((*this)[i] != lhs[i]) {
        return false;
      }
    }
    return true;
  }

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr operator std::string_view() const {
    return std::string_view(this->begin(), this->end());
  }

  [[nodiscard]] constexpr auto containsInvalidChar() const -> bool {
    auto invalid_chars = std::string_view(" \\\"'<>&|$[]");
    if (invalid_chars.contains(this->shortName)) {
      return true;
    }
    for (std::size_t i = 0; i < this->nameLen; i++) {
      if (invalid_chars.contains(this->name[i])) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] constexpr auto hasValidNameLength() const -> bool {
    if (this->shortName == '\0') {
      return true;
    }
    return (N - this->nameLen) == 2;
  }
};

export template <std::size_t N>
ArgName(const char (&)[N]) -> ArgName<N - 1>;

}  // namespace Argo

// generator end here
