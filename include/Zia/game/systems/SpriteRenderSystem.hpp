#pragma once

#include "Zia/game/world/Camera.hpp"
#include "Zia/engine/IRenderer.hpp"
#include "Zia/engine/IAssetManager.hpp"
#include "Zia/engine/IEntityManager.hpp"

namespace zia {
    class SpriteRenderSystem {
    public:
        // Render all entities that have a SpriteComponent, using Position/Size components.
        void render(zia::engine::IRenderer& renderer, const Camera& camera, zia::engine::IEntityManager& registry, zia::engine::IAssetManager& assets);
    };
} // namespace Zia
