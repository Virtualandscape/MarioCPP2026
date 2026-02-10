#pragma once

#include "mario/engine/EngineState.hpp"
#include "mario/engine/SystemScheduler.hpp"
#include "mario/ecs/EntityManager.hpp"
#include "mario/input/InputManager.hpp"
#include "mario/render/Renderer.hpp"
#include "mario/resources/AssetManager.hpp"

#include <memory>
#include <vector>


namespace mario::engine {
    // Core engine that owns the loop, renderer, input manager, and resource cache.
    class Engine {
    public:
        Engine() = default;

        virtual ~Engine() = default;

        void initialize();

        void shutdown();

        void run();

        void push_state(std::shared_ptr<EngineState> state);

        void pop_state();

        std::shared_ptr<EngineState> current_state();

        Renderer &renderer();

        InputManager &input();

        AssetManager &assets();

        EntityManager &entity_manager();

        SystemScheduler &system_scheduler();

    protected:
        // Called before the main loop begins so derived games can set an initial state.
        virtual void before_loop();

    private:
        void main_loop();

        bool _running = false;
        std::vector<std::shared_ptr<EngineState> > _states;
        Renderer _renderer;
        InputManager _input;
        AssetManager _assets;
        EntityManager _registry;
        SystemScheduler _scheduler;
    };
} // namespace mario::engine
