#pragma once

#include "Zia/game/world/Camera.hpp"
#include "Zia/engine/ecs/components/BackgroundComponent.hpp"
#include "Zia/engine/ecs/EntityManager.hpp"
#include "Zia/engine/IEntityManager.hpp"
#include "Zia/engine/IRenderer.hpp"
#include "Zia/engine/IAssetManager.hpp"

namespace zia {
    class BackgroundSystem {
    public:
        void render(zia::engine::IRenderer& renderer, const Camera& camera, zia::engine::IAssetManager& assets, const BackgroundComponent& bg);

        // Crée une entité de fond et y attache un BackgroundComponent.
        // Prend le registry en paramètre pour éviter de maintenir une référence interne au système.
        void create_background_entity(zia::engine::IEntityManager& registry, int texture_id, bool preserve_aspect,
                                      BackgroundComponent::ScaleMode scale_mode, float scale_multiplier, float parallax,
                                      bool repeat, bool repeat_x = false, float offset_x = 0.0f, float offset_y = 0.0f);
    };
} // namespace Zia
