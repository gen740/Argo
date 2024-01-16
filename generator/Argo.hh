#pragma once

#include <unistd.h>

#include <array>
#include <cassert>
#include <charconv>
#include <concepts>
#include <cstring>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#define ARGO_ALWAYS_INLINE __attribute__((always_inline))

// fetch { Argo/ArgoExceptions.cc }
// fetch { Argo/ArgoTypeTraits.cc }
// fetch { Argo/ArgoValidation.cc }
// fetch { Argo/ArgoArgName.cc }
// fetch { Argo/ArgoArg.cc }
// fetch { Argo/ArgoInitializer.cc }
// fetch { Argo/ArgoHelpGenerator.cc }
// fetch { Argo/ArgoMetaLookup.cc }
// fetch { Argo/ArgoMetaAssigner.cc }
// fetch { Argo/ArgoMetaParse.cc }
// fetch { Argo/ArgoParser.cc }
// fetch { Argo/ArgoParserImpl.cc }
