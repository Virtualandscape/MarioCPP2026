// Implements the SpriteRenderSystem, which renders all sprite entities based on their Position, Size, and Sprite components.
// Iterates over entities with sprite data and draws shapes (rectangles or ellipses) at world positions within the camera view.

#include "mario/systems/SpriteRenderSystem.hpp"
#include "mario/ecs/components/SpriteComponent.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"

namespace mario {
    // Renders all sprites by querying entities with all sprite-related components (SpriteComponent, PositionComponent, SizeComponent).
    // This follows the ECS pattern: systems operate on entities with required component combinations.
    void SpriteRenderSystem::render(Renderer& renderer, const Camera& camera, EntityManager& registry) {
        // Set camera for world-space rendering
        renderer.set_camera(camera.x(), camera.y());

        // Query all entities that have sprite components: position, size, and appearance
        static thread_local std::vector<EntityID> renderables;
        renderables.clear();
        registry.get_entities_with<SpriteComponent, PositionComponent, SizeComponent>(renderables);

        // Render each sprite entity
        for (auto entity : renderables) {
            auto* sprite = registry.get_component<SpriteComponent>(entity);
            auto* pos = registry.get_component<PositionComponent>(entity);
            auto* size = registry.get_component<SizeComponent>(entity);

            // All components are guaranteed to exist from the multi-component query
            if (sprite && pos && size) {
                if (sprite->shape == SpriteComponent::Shape::Rectangle) {
                    renderer.draw_rect(pos->x, pos->y, size->width, size->height, sprite->color);
                } else {
                    renderer.draw_ellipse(pos->x, pos->y, size->width, size->height, sprite->color);
                }
            }
        }
    }
}
