#pragma once

#include "mario/engine/IRenderer.hpp"
#include "mario/render/Renderer.hpp"

#include <memory>

namespace mario::engine::adapters {
    // Simple adapter that exposes the concrete mario::Renderer via the IRenderer interface.
    class RendererAdapter : public IRenderer {
    public:
        explicit RendererAdapter(std::shared_ptr<mario::Renderer> r) : _renderer(std::move(r)) {}
        ~RendererAdapter() override = default;

        // IRenderer implementation
        sf::RenderWindow &window() override { return _renderer->window(); }
        void begin_frame() override { _renderer->begin_frame(); }
        void end_frame() override { _renderer->end_frame(); }

        void set_camera(float x, float y) override { _renderer->set_camera(x, y); }
        sf::Vector2f viewport_size() const override { return _renderer->viewport_size(); }
        void set_camera_scale(float s) override { _renderer->set_camera_scale(s); }
        float camera_scale() const override { return _renderer->camera_scale(); }

        void draw_rect(float x, float y, float width, float height, sf::Color color) override { _renderer->draw_rect(x,y,width,height,color); }
        void draw_sprite(const sf::Texture& texture, float x, float y, float width, float height, const sf::IntRect& texture_rect) override { _renderer->draw_sprite(texture,x,y,width,height,texture_rect); }
        void draw_sprite(int sprite_id, float x, float y) override { _renderer->draw_sprite(sprite_id,x,y); }
        void draw_text(const std::string& text, float x, float y, unsigned int size, sf::Color color) override { _renderer->draw_text(text,x,y,size,color); }
        void draw_ellipse(float x, float y, float width, float height, sf::Color color) override { _renderer->draw_ellipse(x,y,width,height,color); }
        void draw_bbox(float x, float y, float width, float height, sf::Color color, float thickness) override { _renderer->draw_bbox(x,y,width,height,color,thickness); }

        void toggle_debug_bboxes() override { _renderer->toggle_debug_bboxes(); }
        bool is_debug_bboxes_enabled() const override { return _renderer->is_debug_bboxes_enabled(); }
        bool is_open() const override { return _renderer->is_open(); }

        [[nodiscard]] std::shared_ptr<mario::Renderer> underlying() const { return _renderer; }

    private:
        std::shared_ptr<mario::Renderer> _renderer;
    };
} // namespace mario::engine::adapters

