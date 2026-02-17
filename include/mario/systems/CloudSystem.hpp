#pragma once

#include "mario/engine/EntityManagerFacade.hpp"
#include "mario/world/Camera.hpp"
#include "mario/engine/IRenderer.hpp"
#include "mario/engine/IAssetManager.hpp"

namespace mario {
    class CloudSystem {
    public:
        void initialize(mario::engine::IAssetManager& assets, mario::engine::EntityManagerFacade& registry);
        void update(mario::engine::EntityManagerFacade& registry, float dt);
        void render(mario::engine::IRenderer& renderer, const Camera& camera, mario::engine::IAssetManager& assets, mario::engine::EntityManagerFacade& registry);
    };
} // namespace mario
