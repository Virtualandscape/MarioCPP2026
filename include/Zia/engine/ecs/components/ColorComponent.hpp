#pragma once

#include <cstdint>

namespace zia {
    // Color component usable by the editor and for enemies.
    // Stores RGBA channels in the 0.0f - 1.0f range for easy use with ImGui and SFML.
    struct ColorComponent {
        float r = 1.0f; // red channel
        float g = 1.0f; // green channel
        float b = 1.0f; // blue channel
        float a = 1.0f; // alpha channel
    };
} // namespace zia

