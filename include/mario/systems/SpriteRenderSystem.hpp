#pragma once

#include "mario/render/Renderer.hpp"
#include "mario/world/Camera.hpp"
#include "mario/ecs/EntityManager.hpp"
#include "mario/resources/AssetManager.hpp"

namespace mario {
    class SpriteRenderSystem {
    public:
        // Render all entities that have a SpriteComponent, using Position/Size components.
        void render(Renderer& renderer, const Camera& camera, EntityManager& registry, AssetManager& assets);
    };
} // namespace mario
