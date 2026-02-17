#pragma once

#include "mario/engine/IEntityManager.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"

namespace mario {
    class Camera;

    // ECS system responsible for camera positioning and viewport management.
    // Follows dependency injection pattern and uses composition over inheritance.
    // Updates camera position to follow a target entity based on its Position and Size components.
    class CameraSystem {
    public:
        CameraSystem() = default;

        // Updates camera viewport and positioning to follow the target entity.
        // Parameters:
        //   - registry: ECS manager containing entity components
        //   - camera: Camera to update (required for rendering)
        //   - dt: delta time in seconds for camera smoothing
        //   - viewport_w/viewport_h: current viewport dimensions
        //   - target: EntityID of the entity to follow (0 = no target)
        void update(mario::engine::IEntityManager& registry, Camera& camera, float dt, float viewport_w, float viewport_h, EntityID target = 0);

        // Initializes camera viewport and target position with optional offset animation.
        // Used when entering a new level to position camera with entrance animation.
        // Parameters:
        //   - registry: ECS manager containing entity components
        //   - camera: Camera to initialize (required for rendering)
        //   - viewport_w/viewport_h: current viewport dimensions
        //   - target: EntityID of the entity to follow (0 = no target)
        //   - initial_offset_x/y: initial position offset for entrance animation
        void initialize(mario::engine::IEntityManager& registry, Camera& camera, float viewport_w, float viewport_h, EntityID target = 0,
                        float initial_offset_x = 0.0f, float initial_offset_y = 0.0f);

    private:
        // Helper method: updates camera target position from entity components
        void _update_camera_target(Camera& camera, const PositionComponent& pos, const SizeComponent& size);

        // Helper method: initializes camera target with offset animation
        void _initialize_camera_target(Camera& camera, const PositionComponent& pos, const SizeComponent& size,
                                       float viewport_w, float viewport_h,
                                       float initial_offset_x, float initial_offset_y);
    };
} // namespace mario
