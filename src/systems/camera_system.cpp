#include "mario/systems/CameraSystem.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"

namespace mario {
    void CameraSystem::update(EntityManager& registry, Camera& camera, EntityID target, float dt, float viewport_w, float viewport_h) {
        // Always set viewport
        camera.set_viewport(viewport_w, viewport_h);

        // If there's no valid target, just call camera.update to process smoothing/animations
        if (target == 0) {
            camera.update(dt);
            return;
        }

        // Try to get position and size components for the target entity
        auto* pos = registry.get_component<PositionComponent>(target);
        auto* size = registry.get_component<SizeComponent>(target);
        if (!pos || !size) {
            // No valid target components; still update camera smoothing
            camera.update(dt);
            return;
        }

        // Compute center of the target and set as camera target
        const float center_x = pos->x + size->width * 0.5f;
        const float center_y = pos->y + size->height * 0.5f;
        camera.set_target(center_x, center_y);

        // Finally update camera (smoothing handled by Camera::update)
        camera.update(dt);
    }

    void CameraSystem::initialize(EntityManager& registry, Camera& camera, EntityID target, float viewport_w, float viewport_h,
                                  float initial_offset_x, float initial_offset_y) {
        // Set viewport size
        camera.set_viewport(viewport_w, viewport_h);

        if (target == 0) {
            // No target, nothing to center on; keep camera at its default
            return;
        }

        auto* pos = registry.get_component<PositionComponent>(target);
        auto* size = registry.get_component<SizeComponent>(target);
        if (!pos || !size) {
            return;
        }

        // Compute center of the target and apply optional initial offset
        const float center_x = pos->x + size->width * 0.5f;
        const float center_y = pos->y + size->height * 0.5f;

        // Apply the offset to the camera position (rather than target) to preserve smoothing animation when entering.
        const float target_x = center_x - viewport_w * 0.5f;
        const float target_y = center_y - viewport_h * 0.5f;

        camera.set_target(center_x, center_y);
        camera.set_position(target_x + initial_offset_x, target_y + initial_offset_y);
    }
} // namespace mario
