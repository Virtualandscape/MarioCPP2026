#pragma once

namespace mario::engine {
    // Minimal interface for a renderer used by the engine. Add methods as the engine needs them.
    class IRenderer {
    public:
        virtual ~IRenderer() = default;
    };
} // namespace mario::engine

