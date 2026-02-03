#pragma once

#include "mario/render/Renderer.hpp"
#include "mario/world/Camera.hpp"
#include "mario/ecs/components/BackgroundComponent.hpp"

namespace mario {
    class AssetManager;

    class BackgroundSystem {
    public:
        void render(Renderer& renderer, const Camera& camera, AssetManager& assets, const BackgroundComponent& bg);
    };
} // namespace mario
