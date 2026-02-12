#pragma once

#include "mario/ecs/EntityManager.hpp"
#include "mario/render/Renderer.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/world/World.hpp"

namespace mario::systems::render {
    // Updates camera viewport and positioning to follow the target entity.
    void update_camera(EntityManager& registry, Camera& camera, float dt, float viewport_w, float viewport_h, EntityID target);

    // Initializes camera viewport and target position with optional offset animation.
    void init_camera(EntityManager& registry, Camera& camera, float viewport_w, float viewport_h, EntityID target,
                      float initial_offset_x, float initial_offset_y);

    // Updates object animations based on velocity and state.
    void update_animations(EntityManager& registry, float dt);

    // Renders all sprite entities based on their components.
    void render_sprites(Renderer& renderer, const Camera& camera, EntityManager& registry, AssetManager& assets);

    // Renders the background with parallax and tiling.
    void render_backgrounds(EntityManager& registry, Renderer& renderer, AssetManager& assets, const Camera& camera);

    // Renders decorative cloud entities.
    void render_clouds(Renderer& renderer, const Camera& camera, AssetManager& assets, EntityManager& registry);

    // Renders debug information (hitboxes, etc).
    void render_debug(EntityManager& registry, Renderer& renderer, const Camera& camera);
}
