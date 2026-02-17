#pragma once

namespace zia {
    struct BackgroundComponent {
        int texture_id = 0; // Key in AssetManager
        bool preserve_aspect = true;
        enum class ScaleMode { Fit, Fill } scale_mode = ScaleMode::Fit;
        // Multiplier applied to the computed scale (1.0 = no extra zoom). Use >1 to zoom in.
        float scale_multiplier = 1.0f;
        float parallax = 0.0f; // 0 = follows camera, 1 = fixed to world
        float offset_x = 0.0f; // Offset along the X axis
        float offset_y = 0.0f; // Offset along the Y axis
        bool repeat = true; // Repeat both horizontally and vertically
        bool repeat_x = false; // Repeat only horizontally (horizontal)
    };
} // namespace Zia
