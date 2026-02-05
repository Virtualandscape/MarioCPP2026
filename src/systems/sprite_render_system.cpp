#include "mario/systems/SpriteRenderSystem.hpp"
#include "mario/ecs/components/SpriteComponent.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"

namespace mario {
    void SpriteRenderSystem::render(Renderer& renderer, const Camera& camera, EntityManager& registry) {
        // Prepare camera
        renderer.set_camera(camera.x(), camera.y());

        static thread_local std::vector<EntityID> renderables;
        renderables.clear();
        registry.get_entities_with<SpriteComponent>(renderables);

        for (auto entity : renderables) {
            auto* pos = registry.get_component<PositionComponent>(entity);
            auto* size = registry.get_component<SizeComponent>(entity);
            auto* sprite = registry.get_component<SpriteComponent>(entity);

            if (pos && size && sprite) {
                if (sprite->shape == SpriteComponent::Shape::Rectangle) {
                    renderer.draw_rect(pos->x, pos->y, size->width, size->height, sprite->color);
                } else {
                    renderer.draw_ellipse(pos->x, pos->y, size->width, size->height, sprite->color);
                }
            }
        }
    }
}
