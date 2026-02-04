#include "mario/systems/BackgroundSystem.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/ecs/components/BackgroundComponent.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <algorithm>
#include <cmath>

namespace mario {

    void BackgroundSystem::render(Renderer& renderer, const Camera& camera, AssetManager& assets, const BackgroundComponent& bg) {
        const auto viewport = renderer.viewport_size();
        sf::Texture* tex = assets.get_mutable_texture(bg.texture_id);
        if (!tex) {
            return; // nothing to draw
        }

        // Enable/disable repeating based on component settings
        if (tex->isRepeated() != bg.repeat) {
            tex->setRepeated(bg.repeat);
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

        float factor = 1.0f - bg.parallax;
        float offset_x = cam_x * factor;
        float offset_y = cam_y * factor;

        sf::Sprite sprite(*tex);
        if (!bg.repeat) {
            float pos_x = (vw - dst_w) * 0.5f + bg.offset_x;
            float pos_y = (vh - dst_h) * 0.5f + bg.offset_y;

            sprite.setScale({scaleX, scaleY});
            
            // Draw in world space but shifted by parallax
            sprite.setPosition({pos_x + offset_x, pos_y + offset_y});
            renderer.window().draw(sprite);
        } else {
            // Tiled background
            
            // Calculate where the "origin" of the background is in world space
            float origin_x = (vw - dst_w) * 0.5f + bg.offset_x + offset_x;
            float origin_y = (vh - dst_h) * 0.5f + bg.offset_y + offset_y;

            // Shift origin to be within [cam_x - dst_w, cam_x]
            float shift_x = std::floor((origin_x - cam_x) / dst_w) * dst_w;
            float shift_y = std::floor((origin_y - cam_y) / dst_h) * dst_h;
            
            float start_x = origin_x - shift_x; 
            float start_y = origin_y - shift_y;
            
            // Ensure we start before the camera view
            while (start_x > cam_x) start_x -= dst_w;
            while (start_y > cam_y) start_y -= dst_h;
            
            // How much we need to cover from start_x to cam_x + vw
            float needed_w = (cam_x + vw) - start_x;
            float needed_h = (cam_y + vh) - start_y;
            
            sprite.setScale({scaleX, scaleY});
            sprite.setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(needed_w / scaleX + 1), static_cast<int>(needed_h / scaleY + 1)}));
            sprite.setPosition({start_x, start_y});
            
            renderer.window().draw(sprite);
        }
    }
} // namespace mario
