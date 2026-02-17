#pragma once

#include "Zia/engine/Scene.hpp"
#include "Zia/game/ui/HUD.hpp"
#include <string>
#include <vector>

namespace zia {
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

        // Show the settings ImGui window
        bool _show_settings = false;
        // Resolution choices: index into this list
        int _ui_resolution_index = 0; // 0 = 800x600, 1 = 1024x768, 2 = 1280x720, 3 = fullscreen
        bool _ui_fullscreen = false;
        float _ui_master_volume = 1.0f;
    };
} // namespace Zia

