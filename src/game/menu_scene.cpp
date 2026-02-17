// Implements the MenuScene class, which manages the main menu, level selection, and input handling for menu navigation.
// Handles rendering menu options and transitioning to the PlayScene when a level is selected.

#include "Zia/game/MenuScene.hpp"
#include "Zia/game/MarioGame.hpp"
#include "Zia/game/PlayScene.hpp"
#include "Zia/game/helpers/Constants.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include "Zia/engine/EngineConfig.hpp"
#include <imgui.h>

namespace zia {
    // Constructor initializes the menu with available levels and prepares text objects for rendering.
    MenuScene::MenuScene(Game& game) : _game(game) {
        _levels.assign(zia::constants::LEVEL_PATHS.begin(), zia::constants::LEVEL_PATHS.end());
        for (size_t i = 0; i < _levels.size(); ++i) {
            _level_texts.emplace_back(_game.renderer());
            _level_texts.back().set_string("Level " + std::to_string(i + 1));
            _level_texts.back().set_size(24);
        }

        // Initialize UI cached values from settings
        if (auto s = _game.settings()) {
            _ui_width = s->window_width();
            _ui_height = s->window_height();
            _ui_fullscreen = s->fullscreen();
            _ui_master_volume = s->master_volume();
        }
    }

    // Called when entering the menu scene.
    void MenuScene::on_enter() {
        _running = true;
    }

    // Called when exiting the menu scene.
    void MenuScene::on_exit() {
    }

    // Handles input for navigating the menu and selecting a level.
    void MenuScene::update(float dt) {
        (void)dt;
        _game.input().poll();

        bool up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
        bool down = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
        bool enter = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

        if (up && !_up_pressed) {
            _selected_index = (_selected_index - 1 + static_cast<int>(_levels.size())) % static_cast<int>(_levels.size());
        }
        if (down && !_down_pressed) {
            _selected_index = (_selected_index + 1) % static_cast<int>(_levels.size());
        }
        if (enter && !_enter_pressed) {
            // Start the selected level by pushing a new PlayScene.
            // Keep behavior: if user presses Enter on menu, start the selected level.
            _game.push_scene(std::make_shared<PlayScene>(_game, _levels[_selected_index]));
            return;
        }

        _up_pressed = up;
        _down_pressed = down;
        _enter_pressed = enter;

        // Mouse handling
        sf::Vector2i mouse_pos = sf::Mouse::getPosition(_game.renderer().window());
        const auto mx = static_cast<float>(mouse_pos.x);
        const auto my = static_cast<float>(mouse_pos.y);
        for (size_t i = 0; i < _levels.size(); ++i) {
            float x = 300;
            float y = 150 + static_cast<float>(i) * 100;
            float w = 200;
            float h = 50;

            if (mx >= x && mx <= x + w && my >= y && my <= y + h) {
                _selected_index = static_cast<int>(i);
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    _game.push_scene(std::make_shared<PlayScene>(_game, _levels[_selected_index]));
                    return;
                }
            }
        }

        if (_game.input().is_pressed(InputManager::Action::Escape)) {
            _running = false;
        }
    }

    // Renders the menu, highlighting the selected level.
    void MenuScene::render() {
        // Render only: the Game loop handles frame begin/end.

        // Draw background
         _game.renderer().draw_rect(0, 0, 800, 480, sf::Color(20, 20, 20));

         // Draw level options as rectangles and text
         for (size_t i = 0; i < _levels.size(); ++i) {
             sf::Color rect_color = (static_cast<int>(i) == _selected_index) ? sf::Color(50,50,50) : sf::Color(40,40,40);
             float x = 300;
             float y = 150 + static_cast<float>(i) * 100;

             // Selection/hover handled in update(); render only paints the rectangle and text.
             // Draw rectangle
             _game.renderer().draw_rect(x, y, 200, 50, rect_color);

             auto& text = _level_texts[i];
             text.set_position(x + 50, y + 10);
             text.set_color(sf::Color::White);
             text.render();

             // Draw a small indicator for selection
             if (static_cast<int>(i) == _selected_index) {
                 _game.renderer().draw_rect(270, y + 10, 20, 30, sf::Color::Red);
             }
         }

        // ImGui top main menu bar: Play + Settings
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Play")) {
                if (ImGui::MenuItem("Start Level 1")) {
                    // Always start the game at level 1
                    if (!_levels.empty()) {
                        // Convert constexpr string_view to std::string explicitly to match PlayScene ctor
                        _game.push_scene(std::make_shared<PlayScene>(_game, std::string(zia::constants::LEVEL_PATHS.at(0))));
                        ImGui::EndMenu();
                        ImGui::EndMainMenuBar();
                        return; // Early return to avoid drawing settings window in same frame
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Settings")) {
                if (ImGui::MenuItem("Open Settings")) {
                    _show_settings = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Settings window
        if (_show_settings) {
            ImGui::Begin("Settings", &_show_settings, ImGuiWindowFlags_AlwaysAutoResize);

            // Synchronize cached values with settings manager at window open
            static bool initialized = false;
            if (!initialized) {
                if (auto s = _game.settings()) {
                    _ui_width = s->window_width();
                    _ui_height = s->window_height();
                    _ui_fullscreen = s->fullscreen();
                    _ui_master_volume = s->master_volume();
                }
                initialized = true;
            }

            // Window size
            ImGui::InputInt("Width", &_ui_width);
            ImGui::InputInt("Height", &_ui_height);
            ImGui::Checkbox("Fullscreen", &_ui_fullscreen);

            // Audio
            ImGui::SliderFloat("Master Volume", &_ui_master_volume, 0.0f, 1.0f);

            if (ImGui::Button("Apply")) {
                if (auto s = _game.settings()) {
                    s->set_window_size(_ui_width, _ui_height);
                    s->set_fullscreen(_ui_fullscreen);
                    s->set_master_volume(_ui_master_volume);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                _show_settings = false;
                initialized = false;
            }

            ImGui::End();
        }

        // end_frame() is called by Game::main_loop()
     }

    bool MenuScene::is_running() const { return _running && _game.renderer().is_open(); }
} // namespace Zia

