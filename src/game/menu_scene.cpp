// Implements the MenuScene class, which manages the main menu, level selection, and input handling for menu navigation.
// Handles rendering menu options and transitioning to the PlayScene when a level is selected.

#include "mario/game/MenuScene.hpp"
#include "mario/core/Game.hpp"
#include "mario/core/PlayScene.hpp"
#include "mario/helpers/Constants.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

namespace mario {
    // Constructor initializes the menu with available levels and prepares text objects for rendering.
    MenuScene::MenuScene(Game& game) : _game(game) {
        _levels.assign(mario::constants::LEVEL_PATHS.begin(), mario::constants::LEVEL_PATHS.end());
        for (size_t i = 0; i < _levels.size(); ++i) {
            _level_texts.emplace_back(_game.renderer());
            _level_texts.back().set_string("Level " + std::to_string(i + 1));
            _level_texts.back().set_size(24);
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

        // end_frame() is called by Game::main_loop()
     }

    bool MenuScene::is_running() const { return _running && _game.renderer().is_open(); }
} // namespace mario

