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
#include <iostream>
#include <SFML/Graphics/Sprite.hpp>
// Editor UI toggle
#include "Zia/editor/EditorUI.hpp"

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
            // Determine resolution index from settings
            const int w = s->window_width();
            const int h = s->window_height();
            _ui_master_volume = s->master_volume();
            _ui_fullscreen = s->fullscreen();
            if (_ui_fullscreen) {
                _ui_resolution_index = 3;
            } else if (w == 800 && h == 600) {
                _ui_resolution_index = 0;
            } else if (w == 1024 && h == 768) {
                _ui_resolution_index = 1;
            } else if (w == 1280 && h == 720) {
                _ui_resolution_index = 2;
            } else {
                // Unknown resolution: pick closest (default 0)
                _ui_resolution_index = 0;
            }
        }
    }

    // Called when entering the menu scene.
    void MenuScene::on_enter() {
        _running = true;

        // Load menu background images (one per resolution choice). Use AssetManager IDs starting at MENU_BACKGROUND_TEXTURE_ID.
        using namespace zia::constants;
        const std::array<std::pair<int, const char*>, 3> bg_files = {
            std::make_pair(MENU_BACKGROUND_TEXTURE_ID + 0, "assets/Backgrounds/menu_background-800x600.png"),
            std::make_pair(MENU_BACKGROUND_TEXTURE_ID + 1, "assets/Backgrounds/menu_background-1024x768.png"),
            std::make_pair(MENU_BACKGROUND_TEXTURE_ID + 2, "assets/Backgrounds/menu_background-1280x720.png")
        };

        for (const auto &p : bg_files) {
            const int id = p.first;
            const std::string path = p.second;
            // Avoid redundant loads
            if (!_game.assets().has_texture(id)) {
                if (!_game.assets().load_texture(id, path)) {
                    std::cerr << "MenuScene: failed to load background texture '" << path << "' (id=" << id << ")\n";
                }
            }
        }
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

        // Draw background: use menu image (fill) when enabled and available, otherwise fallback to solid rect.
        const auto viewport = _game.renderer().viewport_size();

        // Choose texture variant based on current UI resolution index. For fullscreen, fall back to the largest provided.
        int tex_index = _ui_resolution_index;
        if (tex_index < 0) tex_index = 0;
        if (tex_index > 2) tex_index = 2; // 3 == fullscreen -> use 1280x720 variant

        const int tex_id = zia::constants::MENU_BACKGROUND_TEXTURE_ID + tex_index;

        if (_use_menu_background) {
            auto tex_ptr = _game.assets().get_texture(tex_id);
            if (tex_ptr) {
                // Draw the texture stretched to fill the viewport (fill behavior requested).
                // Draw in UI/screen space (default view) so camera/world transforms do not affect scaling.
                sf::RenderWindow &window = _game.renderer().window();
                auto old_view = window.getView();

                // Query window and texture sizes first
                const auto win_size = window.getSize();
                const float win_w = static_cast<float>(win_size.x);
                const float win_h = static_cast<float>(win_size.y);
                const auto tex_size = tex_ptr->getSize();
                const float tex_w = static_cast<float>(tex_size.x);
                const float tex_h = static_cast<float>(tex_size.y);

                // Use an explicit UI view mapped 1:1 to window pixels to avoid any camera/world transforms
                sf::View ui_view;
                ui_view.setSize({win_w, win_h});
                ui_view.setCenter({win_w * 0.5f, win_h * 0.5f});
                window.setView(ui_view);

                sf::Sprite sprite(*tex_ptr);
                // Ensure origin is top-left; sprite uses full texture by default so explicit texture rect is unnecessary.
                sprite.setOrigin({0.0f, 0.0f});

                // Scale to window pixel size (fill)
                if (tex_w > 0.0f && tex_h > 0.0f) {
                    const float scale_x = win_w / tex_w;
                    const float scale_y = win_h / tex_h;
                    sprite.setScale({ scale_x, scale_y });
                }
                sprite.setPosition({0.0f, 0.0f});
                window.draw(sprite);
                window.setView(old_view);
             } else {
                 // Fallback: solid color covering the whole viewport
                 _game.renderer().draw_rect(0.0f, 0.0f, viewport.x, viewport.y, sf::Color(20, 20, 20));
             }
         } else {
            _game.renderer().draw_rect(0.0f, 0.0f, viewport.x, viewport.y, sf::Color(20, 20, 20));
         }

        // Draw level options as rectangles and text
        /*for (size_t i = 0; i < _levels.size(); ++i) {
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
        }*/

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
            // Editor menu: open/close integrated Scene Editor
            if (ImGui::BeginMenu("Editor")) {
                bool vis = zia::editor::is_editor_visible();
                if (ImGui::MenuItem("Open Scene Editor", nullptr, vis)) {
                    zia::editor::set_editor_visible(!vis);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Settings window
        if (_show_settings) {
            ImGui::Begin("Settings", &_show_settings, ImGuiWindowFlags_AlwaysAutoResize);

            // Resolution options
            const char* resolutions[] = { "800 x 600", "1024 x 768", "1280 x 720", "Fullscreen" };
            ImGui::Combo("Resolution", &_ui_resolution_index, resolutions, IM_ARRAYSIZE(resolutions));

            // Fullscreen checkbox kept in sync with resolution selection for clarity
            bool fs = (_ui_resolution_index == 3) ? true : _ui_fullscreen;
            if (ImGui::Checkbox("Fullscreen (override)", &fs)) {
                _ui_fullscreen = fs;
                if (_ui_fullscreen) _ui_resolution_index = 3;
            }

            // Toggle to enable/disable menu background image
            ImGui::Checkbox("Use menu background image", &_use_menu_background);

            // Audio
            ImGui::SliderFloat("Master Volume", &_ui_master_volume, 0.0f, 1.0f);

            if (ImGui::Button("Apply")) {
                if (auto s = _game.settings()) {
                    if (_ui_resolution_index == 0) {
                        s->set_fullscreen(false);
                        s->set_window_size(800, 600);
                    } else if (_ui_resolution_index == 1) {
                        s->set_fullscreen(false);
                        s->set_window_size(1024, 768);
                    } else if (_ui_resolution_index == 2) {
                        s->set_fullscreen(false);
                        s->set_window_size(1280, 720);
                    } else if (_ui_resolution_index == 3) {
                        s->set_fullscreen(true);
                    }
                    s->set_master_volume(_ui_master_volume);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                _show_settings = false;
            }

            ImGui::End();
        }

        // end_frame() is called by Game::main_loop()
     }

    bool MenuScene::is_running() const { return _running && _game.renderer().is_open(); }
} // namespace Zia

