#pragma once

#include "mario/core/Scene.hpp"
#include "mario/ui/HUD.hpp"
#include <string>
#include <vector>

namespace mario {
    class Game;

    // Scene representing the main menu and level selection.
    class MenuScene : public Scene {
    public:
        MenuScene(Game& game);

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
        bool _up_pressed = false;
        bool _down_pressed = false;
        bool _enter_pressed = false;
    };
} // namespace mario

