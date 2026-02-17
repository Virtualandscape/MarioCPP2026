#include "mario/systems/DebugDrawSystem.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/ecs/components/TypeComponent.hpp"
#include "mario/world/Camera.hpp"

#include <SFML/Graphics.hpp>
#include <vector>

namespace mario {
    void DebugDrawSystem::render(mario::engine::IRenderer& renderer, const Camera& camera, mario::engine::IEntityManager& registry) {
        // Only draw if debug bboxes enabled
        if (!renderer.is_debug_bboxes_enabled()) return;

        // Ensure the renderer uses the camera view
        renderer.set_camera(camera.x(), camera.y());

        static thread_local std::vector<EntityID> entities;
        entities.clear();
        registry.get_entities_with<PositionComponent, SizeComponent>(entities);

        for (auto entity: entities) {
            auto pos_opt = registry.get_component<PositionComponent>(entity);
            auto size_opt = registry.get_component<SizeComponent>(entity);
            if (!pos_opt || !size_opt) continue;
            auto& pos = pos_opt->get();
            auto& size = size_opt->get();

            sf::Color color = sf::Color(180, 180, 180);
            if (auto type_opt = registry.get_component<TypeComponent>(entity)) {
                auto& type = type_opt->get();
                if (type.type == EntityTypeComponent::Player) {
                    color = sf::Color::Red;
                } else {
                    // enemies & others -> gray
                    color = sf::Color(128, 128, 128);
                }
            }

            // Draw world-space bounding box; renderer translates with camera.
            renderer.draw_bbox(pos.x, pos.y, size.width, size.height, color, 1.0f);
        }
    }
}
