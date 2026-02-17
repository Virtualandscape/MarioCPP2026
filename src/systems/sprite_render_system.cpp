// Implements the SpriteRenderSystem, which renders all sprite entities based on their Position, Size, and Sprite components.
// Iterates over entities with sprite data and draws shapes (rectangles or ellipses) at world positions within the camera view.

#include "mario/systems/SpriteRenderSystem.hpp"
#include "mario/ecs/components/SpriteComponent.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"

namespace mario {
    // Renders all sprites by querying entities with all sprite-related components (SpriteComponent, PositionComponent, SizeComponent).
    // This follows the ECS pattern: systems operate on entities with required component combinations.
    void SpriteRenderSystem::render(mario::engine::IRenderer& renderer, const Camera& camera, mario::engine::EntityManagerFacade& registry, mario::engine::IAssetManager& assets) {
        // Set camera for world-space rendering
        renderer.set_camera(camera.x(), camera.y());

        // Query all entities that have sprite components: position, size, and appearance
        static thread_local std::vector<EntityID> renderables;
        renderables.clear();
        registry.get_entities_with<SpriteComponent, PositionComponent, SizeComponent>(renderables);

        // Render each sprite entity
        for (auto entity : renderables) {
            auto sprite_opt = registry.get_component<SpriteComponent>(entity);
            auto pos_opt = registry.get_component<PositionComponent>(entity);
            auto size_opt = registry.get_component<SizeComponent>(entity);

            // All components are guaranteed to exist from the multi-component query, but keep defensive checks
            if (sprite_opt && pos_opt && size_opt) {
                auto& sprite = sprite_opt->get();
                auto& pos = pos_opt->get();
                auto& size = size_opt->get();

                // Priority 1: Texture rendering
                if (sprite.texture_id != -1) {
                    auto tex = assets.get_texture(sprite.texture_id);
                    if (tex) {
                        // Decide draw size: prefer explicit render_size from SpriteComponent when > 0
                        const float draw_w = (sprite.render_size.x > 0.0f) ? sprite.render_size.x : size.width;
                        const float draw_h = (sprite.render_size.y > 0.0f) ? sprite.render_size.y : size.height;
                        renderer.draw_sprite(*tex, pos.x + sprite.render_offset.x, pos.y + sprite.render_offset.y,
                                             draw_w, draw_h, sprite.texture_rect);
                        continue;
                    }
                }

                // Priority 2: Shape rendering (fallback or if explicitly requested via texture_id == -1)
                if (sprite.shape == SpriteComponent::Shape::Rectangle) {
                    renderer.draw_rect(pos.x, pos.y, size.width, size.height, sprite.color);
                } else if (sprite.shape == SpriteComponent::Shape::Ellipse) {
                    renderer.draw_ellipse(pos.x, pos.y, size.width, size.height, sprite.color);
                }
             }
         }
     }
 }
