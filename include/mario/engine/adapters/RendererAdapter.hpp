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

        [[nodiscard]] std::shared_ptr<mario::Renderer> underlying() const { return _renderer; }

    private:
        std::shared_ptr<mario::Renderer> _renderer;
    };
} // namespace mario::engine::adapters

