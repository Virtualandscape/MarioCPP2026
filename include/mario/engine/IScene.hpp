#pragma once

#include <memory>

namespace mario::engine {
    // Engine-agnostic scene interface. Game-specific scenes should implement this interface
    // (or be adapted) to run on top of the engine.
    class IScene {
    public:
        virtual ~IScene() = default;

        // Called whenever the scene becomes the active scene.
        virtual void on_enter() = 0;

        // Invoked when the scene is removed or another scene gains focus.
        virtual void on_exit() = 0;

        // Performs per-frame logic updates.
        virtual void update(float dt) = 0;

        // Draws the scene when it is active.
        virtual void render() = 0;

        // Signals whether the scene should continue running.
        [[nodiscard]] virtual bool is_running() const { return true; }
    };
} // namespace mario::engine

