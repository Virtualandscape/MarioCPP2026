#include "mario/systems/BackgroundSystem.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/ecs/components/BackgroundComponent.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <algorithm>
#include <cmath>

namespace mario {

    // Renders the background based on the BackgroundComponent settings, handling parallax, scaling, and tiling.
    void BackgroundSystem::render(Renderer& renderer, const Camera& camera, AssetManager& assets, const BackgroundComponent& bg) {
        // Get viewport dimensions
        const auto viewport = renderer.viewport_size();
        // Retrieve the texture for the background
        sf::Texture* tex = assets.get_mutable_texture(bg.texture_id);
        if (!tex) {
            return; // nothing to draw
        }

        // Enable/disable texture repeating based on component settings
        if (tex->isRepeated() != bg.repeat) {
            tex->setRepeated(bg.repeat);
        }

        // Viewport and texture sizes
        const float vw = viewport.x;
        const float vh = viewport.y;
        const float tw = static_cast<float>(tex->getSize().x);
        const float th = static_cast<float>(tex->getSize().y);

        // Compute initial scale to fit viewport
        float scaleX = vw / tw;
        float scaleY = vh / th;
        if (bg.preserve_aspect) {
            // Adjust scale to preserve the aspect ratio
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

        // Destination size after scaling
        const float dst_w = tw * scaleX;
        const float dst_h = th * scaleY;

        // Camera position for parallax calculation
        float cam_x = camera.x();
        float cam_y = camera.y();

        // Parallax offsets: move background relative to camera
        const float offset_x = -cam_x * bg.parallax;
        const float offset_y = -cam_y * bg.parallax;

        // Switch to default view for viewport-relative drawing
        sf::RenderWindow& window = renderer.window();
        const sf::View old_view = window.getView();
        window.setView(window.getDefaultView());

        sf::Sprite sprite(*tex);
        if (!bg.repeat) {
            // Single background image, centered with parallax
            float pos_x = (vw - dst_w) * 0.5f + bg.offset_x + offset_x;
            float pos_y = (vh - dst_h) * 0.5f + bg.offset_y + offset_y;

            sprite.setScale({scaleX, scaleY});
            sprite.setPosition({pos_x, pos_y});
            window.draw(sprite);
        } else if (bg.repeat_x) {
            // Repeat horizontally, fixed at bottom (no vertical parallax)
            float y = vh - dst_h + bg.offset_y;
            // Calculate starting x for seamless tiling with parallax
            float start_x = -std::fmod(offset_x + bg.offset_x, dst_w);
            if (start_x > 0) start_x -= dst_w;

            sprite.setScale({scaleX, scaleY});
            sprite.setTextureRect(sf::IntRect({0, 0}, {static_cast<int>(tw), static_cast<int>(th)}));
            // Draw tiles across the viewport width
            for (float x = start_x; x < vw; x += dst_w) {
                sprite.setPosition({x, y});
                window.draw(sprite);
            }
        } else {
            // Tiled background in both directions with parallax
            // Calculate starting positions for seamless tiling
            float start_x = -std::fmod(offset_x + bg.offset_x, dst_w);
            float start_y = -std::fmod(offset_y + bg.offset_y, dst_h);
            if (start_x > 0) start_x -= dst_w;
            if (start_y > 0) start_y -= dst_h;

            sprite.setScale({scaleX, scaleY});
            // Draw tiles across the entire viewport
            for (float x = start_x; x < vw; x += dst_w) {
                for (float y = start_y; y < vh; y += dst_h) {
                    sprite.setPosition({x, y});
                    window.draw(sprite);
                }
            }
        }

        // Restore the original view
        window.setView(old_view);
    }

    // Create a background entity and attach a BackgroundComponent (moved from PlayState)
    void BackgroundSystem::create_background_entity(EntityManager& registry, int texture_id, bool preserve_aspect,
                                                    BackgroundComponent::ScaleMode scale_mode, float scale_multiplier, float parallax,
                                                    bool repeat, bool repeat_x, float offset_x, float offset_y) {
        auto id = registry.create_entity();
        BackgroundComponent bc;
        bc.texture_id = texture_id;
        bc.preserve_aspect = preserve_aspect;
        bc.scale_mode = scale_mode;
        bc.scale_multiplier = scale_multiplier;
        bc.parallax = parallax;
        bc.repeat = repeat;
        bc.repeat_x = repeat_x;
        bc.offset_x = offset_x;
        bc.offset_y = offset_y;
        registry.add_component(id, bc);
    }

} // namespace mario
