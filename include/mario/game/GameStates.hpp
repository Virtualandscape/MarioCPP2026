#pragma once

#include "mario/engine/EngineState.hpp"
#include "mario/ui/HUD.hpp"
#include <string>
#include <vector>
#include <memory>

namespace mario {

    class Game;
    using GameState = engine::EngineState;

    class MenuState : public GameState {
    public:
        MenuState(Game& game);

        void on_enter() override;
        void on_exit() override;
        void update(float dt) override;
        void render() override;
        bool is_running() const override;

    private:
        Game& _game;
        std::vector<std::string> _levels;
        std::vector<Text> _level_texts;
        int _selected_index = 0;
        bool _running = true;
        bool _up_pressed = false, _down_pressed = false, _enter_pressed = false;
    };

    class PauseState : public GameState {
    public:
        void on_enter() override;
        void on_exit() override;
        void update(float dt) override;
        void render() override;
    };

} // namespace mario

