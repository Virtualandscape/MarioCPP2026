#pragma once

#include "mario/ecs/EntityManager.hpp"
#include "mario/world/Camera.hpp"

namespace mario {
    class CameraSystem {
    public:
        // Updates the camera to follow the given target entity (if any) and applies viewport settings.
        // - registry: entity/component storage
        // - camera: the Camera to update
        // - target: EntityID of the entity to follow (0 = none)
        // - dt: delta time in seconds
        // - viewport_w/viewport_h: current viewport size
        static void update(EntityManager& registry, Camera& camera, EntityID target, float dt, float viewport_w, float viewport_h);

        // Initializes camera viewport and sets initial target + optional initial offset (used when entering a level).
        // - initial_offset_x/y: applied to the computed camera position to allow an initial offset animation.
        static void initialize(EntityManager& registry, Camera& camera, EntityID target, float viewport_w, float viewport_h,
                        float initial_offset_x = 0.0f, float initial_offset_y = 0.0f);
    };
} // namespace mario
