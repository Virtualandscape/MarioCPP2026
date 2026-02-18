#pragma once

#include <SFML/Graphics.hpp>

#include "Zia/game/helpers/Constants.hpp"
#include "Zia/engine/IRenderer.hpp" // Implement the engine renderer interface

namespace zia {
    // Draw calls, sprites, layers, parallax
    // Make Renderer implement the engine-level IRenderer to remove the need for a separate adapter.
    class Renderer : public zia::engine::IRenderer {
    public:
        Renderer();

        // IRenderer implementation
        // Called at the start of a frame to prepare drawing (clear, set view etc.).
        void begin_frame() override;

        // Called after all drawing is done to present the frame.
        void end_frame() override;

        // Camera and viewport helpers
        void set_camera(float x, float y) override;
        sf::Vector2f viewport_size() const override;
        void set_camera_scale(float s) override { _camera_scale = s; }
        float camera_scale() const override { return _camera_scale; }

        // Draw using a texture reference (avoids raw pointer parameters in public API)
        void draw_sprite(const sf::Texture& texture, float x, float y, float width, float height, const sf::IntRect& texture_rect) override;

        void draw_sprite(int sprite_id, float x, float y) override;

        void draw_rect(float x, float y, float width, float height, sf::Color color) override;

        void draw_text(const std::string& text, float x, float y, unsigned int size, sf::Color color) override;

        // Access the underlying render window for event polling and ImGui integration.
        sf::RenderWindow& window() override { return _window; }

        void draw_ellipse(float x, float y, float width, float height, sf::Color color) override;

        bool is_open() const override;

        // Camera scale controls zoom: multiply world viewport by this factor. Default is TILE_SCALE.
        // set_camera_scale / camera_scale already declared as overrides above.

        // Set/get target number of visible tiles horizontally; when > 0 this takes precedence and the view
        // is computed so that that many tiles fit horizontally in the viewport.
        void set_tiles_visible_width(float tiles) { _camera_tiles_w = tiles; }
        float tiles_visible_width() const { return _camera_tiles_w; }

        // Draw an unfilled outline rectangle in world-space (respects camera view).
        // color: outline color. thickness: outline thickness in pixels.
        void draw_bbox(float x, float y, float width, float height, sf::Color color, float thickness) override;

        // Toggle and query debug bounding boxes rendering.
        void toggle_debug_bboxes() override;
        bool is_debug_bboxes_enabled() const override;

    private:
        sf::RenderWindow _window;
        sf::Font _font;
        sf::Color _clear_color = sf::Color(30, 30, 36);
        float _camera_x = 0.0f;
        float _camera_y = 0.0f;
        // When true, systems will draw entity bounding boxes (debug overlay)
        bool _debug_bboxes = false;
        // Multiplier applied to view size to control zoom in world coordinates. Default is TILE_SCALE.
        float _camera_scale = zia::constants::TILE_SCALE;
        // Target number of tiles visible horizontally; if >0, the renderer computes view size so this many tiles fit across.
        float _camera_tiles_w = 50.0f; // default corresponds to previous 800px/16px = 50 tiles
    };
} // namespace Zia
