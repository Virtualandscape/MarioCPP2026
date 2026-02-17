#pragma once

#include "mario/game/world/Camera.hpp"
#include "mario/engine/IRenderer.hpp"
#include "mario/engine/IAssetManager.hpp"
#include "mario/engine/IEntityManager.hpp"

namespace mario {
    class SpriteRenderSystem {
    public:
        // Render all entities that have a SpriteComponent, using Position/Size components.
        void render(mario::engine::IRenderer& renderer, const Camera& camera, mario::engine::IEntityManager& registry, mario::engine::IAssetManager& assets);
    };
} // namespace mario
