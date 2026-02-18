#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

namespace zia::engine {
    // Minimal interface for a renderer used by the engine. Add methods as the engine needs them.
    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        // Access the underlying render window for event polling and ImGui integration.
        virtual sf::RenderWindow &window() = 0;

        // Called at the start of a frame to prepare drawing (clear, set view etc.).
        virtual void begin_frame() = 0;

        // Called after all drawing is done to present the frame.
        virtual void end_frame() = 0;

        // Camera and viewport helpers
        virtual void set_camera(float x, float y) = 0;
        [[nodiscard]] virtual sf::Vector2f viewport_size() const = 0;
        virtual void set_camera_scale(float s) = 0;
        [[nodiscard]] virtual float camera_scale() const = 0;
        // Allow renderer to reserve a top inset (in screen pixels) that will be excluded from
        // the world viewport size. This is used to reserve UI menu space at the top of the window.
        virtual void set_top_inset_pixels(int px) = 0;
        [[nodiscard]] virtual int top_inset_pixels() const = 0;

        // Drawing helpers used by scenes
        virtual void draw_rect(float x, float y, float width, float height, sf::Color color) = 0;
        virtual void draw_sprite(const sf::Texture& texture, float x, float y, float width, float height, const sf::IntRect& texture_rect) = 0;
        virtual void draw_sprite(int sprite_id, float x, float y) = 0;
        virtual void draw_text(const std::string& text, float x, float y, unsigned int size, sf::Color color) = 0;
        virtual void draw_ellipse(float x, float y, float width, float height, sf::Color color) = 0;
        virtual void draw_bbox(float x, float y, float width, float height, sf::Color color, float thickness) = 0;

        // Debug
        virtual void toggle_debug_bboxes() = 0;
        [[nodiscard]] virtual bool is_debug_bboxes_enabled() const = 0;
        [[nodiscard]] virtual bool is_open() const = 0;
    };
} // namespace Zia::engine
