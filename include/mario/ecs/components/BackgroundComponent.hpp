#pragma once

namespace mario {
    struct BackgroundComponent {
        int texture_id = 0; // clé dans AssetManager
        bool preserve_aspect = true;
        enum class ScaleMode { Fit, Fill } scale_mode = ScaleMode::Fit;
        // Multiplier applied to the computed scale (1.0 = no extra zoom). Use >1 to zoom in.
        float scale_multiplier = 1.0f;
        float parallax = 0.0f; // 0 = follows camera, 1 = fixed to world
        float offset_x = 0.0f;
        float offset_y = 0.0f;
        bool repeat = true;
        bool repeat_x = false; // Répéter uniquement sur X (horizontal)
    };
} // namespace mario
