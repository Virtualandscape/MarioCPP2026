#pragma once

namespace mario {
    // Interface that every game state must implement to integrate with the main loop.
    class Scene {
    public:
        virtual ~Scene() = default;

        // Called whenever the state becomes the active state.
        virtual void on_enter() = 0;

        // Invoked when the state is removed or another state gains focus.
        virtual void on_exit() = 0;

        // Performs per-frame logic updates.
        virtual void update(float dt) = 0;

        // Draws the state when it is active.
        virtual void render() = 0;

        // Signals whether the state should continue running.
        virtual bool is_running() const { return true; }
    };
} // namespace mario
