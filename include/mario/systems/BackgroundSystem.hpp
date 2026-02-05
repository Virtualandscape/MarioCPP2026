#pragma once

#include "mario/render/Renderer.hpp"
#include "mario/world/Camera.hpp"
#include "mario/ecs/components/BackgroundComponent.hpp"
#include "mario/ecs/EntityManager.hpp"

namespace mario {
    class AssetManager;

    class BackgroundSystem {
    public:
        void render(Renderer& renderer, const Camera& camera, AssetManager& assets, const BackgroundComponent& bg);

        // Crée une entité de fond et y attache un BackgroundComponent.
        // Prend le registry en paramètre pour éviter de maintenir une référence interne au système.
        void create_background_entity(EntityManager& registry, int texture_id, bool preserve_aspect,
                                      BackgroundComponent::ScaleMode scale_mode, float scale_multiplier, float parallax,
                                      bool repeat, bool repeat_x = false, float offset_x = 0.0f, float offset_y = 0.0f);
    };
} // namespace mario
