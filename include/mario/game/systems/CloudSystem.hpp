#pragma once

#include "mario/engine/IEntityManager.hpp"
#include "mario/game/world/Camera.hpp"
#include "mario/engine/IRenderer.hpp"
#include "mario/engine/IAssetManager.hpp"

namespace mario {
    class CloudSystem {
    public:
        void initialize(mario::engine::IAssetManager& assets, mario::engine::IEntityManager& registry);
        void update(mario::engine::IEntityManager& registry, float dt);
        void render(mario::engine::IRenderer& renderer, const Camera& camera, mario::engine::IAssetManager& assets, mario::engine::IEntityManager& registry);
    };
} // namespace mario
