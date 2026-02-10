#pragma once


namespace mario::engine {
    // Base interface that any state managed by the engine must implement.
    class EngineState {
    public:
        virtual ~EngineState() = default;

        virtual void on_enter() = 0;

        virtual void on_exit() = 0;

        virtual void update(float dt) = 0;

        virtual void render() = 0;

        virtual bool is_running() const { return true; }
    };
} // namespace mario::engine
