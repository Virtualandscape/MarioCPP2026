// Implements the CameraSystem, which manages camera positioning and viewport management following the ECS pattern.
// Updates camera position to follow a target entity based on its Position and Size components (dependency injection).
// Uses composition over inheritance: camera system delegates to Camera class for state management.

#include "mario/game/systems/CameraSystem.hpp"
#include "mario/game/world/Camera.hpp"
#include "mario/engine/ecs/components/PositionComponent.hpp"
#include "mario/engine/ecs/components/SizeComponent.hpp"

namespace mario {
    // Updates camera viewport and positioning to follow the target entity.
    // Applies camera smoothing and keeps target visible within viewport.
    void CameraSystem::update(mario::engine::IEntityManager& registry, Camera& camera, float dt, float viewport_w, float viewport_h, EntityID target) {
        // Update viewport size and apply any camera animations (damping, smoothing)
        camera.set_viewport(viewport_w, viewport_h);
        camera.update(dt);

        // Early return if no target entity is specified (fixed camera mode)
        if (target == 0) {
            return;
        }

        // Query target entity components using optional reference API
        auto pos_opt = registry.get_component<PositionComponent>(target);
        auto size_opt = registry.get_component<SizeComponent>(target);
        if (pos_opt && size_opt) {
            _update_camera_target(camera, pos_opt->get(), size_opt->get());
        }
    }

    // Initializes camera viewport and target position with optional offset animation.
    // Used when entering a new level to position camera with entrance animation (offset damping).
    void CameraSystem::initialize(mario::engine::IEntityManager& registry, Camera& camera, float viewport_w, float viewport_h, EntityID target,
                                  float initial_offset_x, float initial_offset_y) {
        // Set initial viewport size
        camera.set_viewport(viewport_w, viewport_h);

        // Early return if no target entity is specified
        if (target == 0) {
            return;
        }

        // Query target entity components using optional reference API
        auto pos_opt2 = registry.get_component<PositionComponent>(target);
        auto size_opt2 = registry.get_component<SizeComponent>(target);
        if (pos_opt2 && size_opt2) {
            _initialize_camera_target(camera, pos_opt2->get(), size_opt2->get(), viewport_w, viewport_h, initial_offset_x, initial_offset_y);
        }
    }

    // Helper: Updates camera target position from entity center based on Position and Size components.
    void CameraSystem::_update_camera_target(Camera& camera, const PositionComponent& pos, const SizeComponent& size) {
        // Calculate center of target entity
        const float center_x = pos.x + size.width * 0.5f;
        const float center_y = pos.y + size.height * 0.5f;
        // Smoothly follow target (camera uses internal damping)
        camera.set_target(center_x, center_y);
    }

    // Helper: Initializes camera target with offset animation for entrance effect.
    void CameraSystem::_initialize_camera_target(Camera& camera, const PositionComponent& pos, const SizeComponent& size,
                                                  float viewport_w, float viewport_h,
                                                  float initial_offset_x, float initial_offset_y) {
        // Calculate center of target entity
        const float center_x = pos.x + size.width * 0.5f;
        const float center_y = pos.y + size.height * 0.5f;

        // Calculate camera top-left position to center on target
        const float target_x = center_x - viewport_w * 0.5f;
        const float target_y = center_y - viewport_h * 0.5f;

        // Set target for smoothing animation (camera will damping-interpolate towards it)
        camera.set_target(center_x, center_y);
        // Apply initial offset to preserve entering animation (offset will be damped out)
        camera.set_position(target_x + initial_offset_x, target_y + initial_offset_y);
    }
} // namespace mario
