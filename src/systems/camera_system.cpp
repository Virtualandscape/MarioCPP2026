#include "mario/systems/CameraSystem.hpp"
#include "mario/world/Camera.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"

namespace mario {
    void CameraSystem::update(EntityManager& registry, Camera& camera, float dt, float viewport_w, float viewport_h, EntityID target) {
        // Update viewport and camera animation state
        camera.set_viewport(viewport_w, viewport_h);
        camera.update(dt);

        // Early return if no target entity is specified
        if (target == 0) {
            return;
        }

        // Query entity components required for camera positioning
        const auto* pos_comp = registry.get_component<PositionComponent>(target);
        const auto* size_comp = registry.get_component<SizeComponent>(target);

        // Only update camera target if both required components are present
        if (pos_comp && size_comp) {
            _update_camera_target(camera, *pos_comp, *size_comp);
        }
    }

    void CameraSystem::initialize(EntityManager& registry, Camera& camera, float viewport_w, float viewport_h, EntityID target,
                                  float initial_offset_x, float initial_offset_y) {
        // Set viewport size
        camera.set_viewport(viewport_w, viewport_h);

        // Early return if no target entity is specified
        if (target == 0) {
            return;
        }

        // Query entity components required for camera initialization
        const auto* pos_comp = registry.get_component<PositionComponent>(target);
        const auto* size_comp = registry.get_component<SizeComponent>(target);

        // Only initialize camera target if both required components are present
        if (pos_comp && size_comp) {
            _initialize_camera_target(camera, *pos_comp, *size_comp, viewport_w, viewport_h, initial_offset_x, initial_offset_y);
        }
    }

    void CameraSystem::_update_camera_target(Camera& camera, const PositionComponent& pos, const SizeComponent& size) {
        // Calculate the center position of the target entity
        const float center_x = pos.x + size.width * 0.5f;
        const float center_y = pos.y + size.height * 0.5f;
        camera.set_target(center_x, center_y);
    }

    void CameraSystem::_initialize_camera_target(Camera& camera, const PositionComponent& pos, const SizeComponent& size,
                                                  float viewport_w, float viewport_h,
                                                  float initial_offset_x, float initial_offset_y) {
        // Calculate the center position of the target entity
        const float center_x = pos.x + size.width * 0.5f;
        const float center_y = pos.y + size.height * 0.5f;

        // Calculate camera top-left position to center on target
        const float target_x = center_x - viewport_w * 0.5f;
        const float target_y = center_y - viewport_h * 0.5f;

        // Set target for smoothing animation
        camera.set_target(center_x, center_y);
        // Apply initial offset to preserve entering animation
        camera.set_position(target_x + initial_offset_x, target_y + initial_offset_y);
    }
} // namespace mario
