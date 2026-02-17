#pragma once

namespace zia {
    struct CloudComponent {
        int texture_id = 0; // Key in AssetManager
        enum class Layer { Big, Medium, Small } layer = Layer::Big;
        float speed = 10.0f; // Pixels per second
        float x = 0.0f; // Position x
        float y = 0.0f; // Position y
        float scale = 1.0f; // Scale multiplier
    };
} // namespace Zia
