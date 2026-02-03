#include "mario/systems/BackgroundSystem.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/ecs/components/BackgroundComponent.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <algorithm>

namespace mario {

    void BackgroundSystem::render(Renderer& renderer, const Camera& camera, AssetManager& assets, const BackgroundComponent& bg) {
        const auto viewport = renderer.viewport_size();
        const sf::Texture* tex = assets.get_texture(bg.texture_id);
        if (!tex) {
            return; // nothing to draw
        }

        const float vw = viewport.x;
        const float vh = viewport.y;
        const float tw = static_cast<float>(tex->getSize().x);
        const float th = static_cast<float>(tex->getSize().y);

        // Compute scale
        float scaleX = vw / tw;
        float scaleY = vh / th;
        if (bg.preserve_aspect) {
            float scale = 1.0f;
            if (bg.scale_mode == BackgroundComponent::ScaleMode::Fit) {
                scale = std::min(scaleX, scaleY);
            } else {
                scale = std::max(scaleX, scaleY);
            }
            scaleX = scaleY = scale;
        }

        // Apply optional additional scale multiplier (zoom)
        if (bg.scale_multiplier != 1.0f) {
            scaleX *= bg.scale_multiplier;
            scaleY *= bg.scale_multiplier;
        }

        const float dst_w = tw * scaleX;
        const float dst_h = th * scaleY;

        // Position: center by default, apply parallax relative to camera
        float cam_x = camera.x();
        float cam_y = camera.y();

        float pos_x = (vw - dst_w) * 0.5f + bg.offset_x - cam_x * bg.parallax;
        float pos_y = (vh - dst_h) * 0.5f + bg.offset_y - cam_y * bg.parallax;

        sf::Sprite sprite(*tex);
        sprite.setScale({scaleX, scaleY});
        // For parallax == 0 we want the sprite fixed in window space: set position without adding camera
        if (bg.parallax == 0.0f) {
            sprite.setPosition({pos_x, pos_y});

            // Draw in screen space by temporarily setting view to default
            sf::RenderWindow& window = renderer.window();
            auto old_view = window.getView();
            window.setView(window.getDefaultView());
            window.draw(sprite);
            window.setView(old_view);
        } else {
            // For parallax != 0, position is in world space, so add camera offset only if renderer doesn't already set the view
            sprite.setPosition({pos_x + cam_x, pos_y + cam_y});
            renderer.window().draw(sprite);
        }
    }
} // namespace mario
