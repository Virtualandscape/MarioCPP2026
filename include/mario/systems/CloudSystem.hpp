#pragma once

#include "mario/ecs/EntityManager.hpp"
#include "mario/render/Renderer.hpp"
#include "mario/world/Camera.hpp"

namespace mario {
    class AssetManager;

    class CloudSystem {
    public:
        void initialize(AssetManager& assets, EntityManager& registry);
        void update(EntityManager& registry, float dt);
        void render(Renderer& renderer, const Camera& camera, AssetManager& assets, EntityManager& registry);
    };
} // namespace mario
