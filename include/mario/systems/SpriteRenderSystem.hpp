#pragma once

#include "mario/world/Camera.hpp"
#include "mario/engine/IRenderer.hpp"
#include "mario/engine/IAssetManager.hpp"
#include "mario/engine/EntityManagerFacade.hpp"

namespace mario {
    class SpriteRenderSystem {
    public:
        // Render all entities that have a SpriteComponent, using Position/Size components.
        void render(mario::engine::IRenderer& renderer, const Camera& camera, mario::engine::EntityManagerFacade& registry, mario::engine::IAssetManager& assets);
    };
} // namespace mario
