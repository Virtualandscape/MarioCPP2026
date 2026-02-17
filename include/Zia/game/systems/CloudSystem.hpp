#pragma once

#include "Zia/engine/IEntityManager.hpp"
#include "Zia/game/world/Camera.hpp"
#include "Zia/engine/IRenderer.hpp"
#include "Zia/engine/IAssetManager.hpp"

namespace zia {
    class CloudSystem {
    public:
        void initialize(zia::engine::IAssetManager& assets, zia::engine::IEntityManager& registry);
        void update(zia::engine::IEntityManager& registry, float dt);
        void render(zia::engine::IRenderer& renderer, const Camera& camera, zia::engine::IAssetManager& assets, zia::engine::IEntityManager& registry);
    };
} // namespace Zia
