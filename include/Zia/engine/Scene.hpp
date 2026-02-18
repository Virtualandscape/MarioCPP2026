#pragma once

// Include the canonical engine interface.
#include "IScene.hpp"

namespace zia {
    // Deprecated alias to preserve backward compatibility with code that used `zia::Scene`.
    // Prefer `zia::engine::IScene` (engine-agnostic interface) instead.
    // NOTE: the compiler-specific deprecated attribute caused build errors on some toolchains,
    // so we keep a clear comment here. Consider adding a compiler-specific deprecation macro
    // if you want a compile-time warning in your environment.
    using Scene = zia::engine::IScene;
} // namespace zia
