#include "Zia/game/ui/MainMenuBar.hpp"
#include "Zia/game/helpers/Constants.hpp"
#include "Zia/game/PlayScene.hpp"
#include "Zia/editor/EditorUI.hpp"
#include "Zia/game/systems/InspectorSystem.hpp"
#include "Zia/engine/EngineConfig.hpp"
#include <imgui.h>

namespace zia {
    namespace ui {
        void draw_main_menu_bar(zia::Game &game, bool &show_settings) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Play")) {
                    if (ImGui::MenuItem("Start Level 1")) {
                        // Start level 1 if available
                        game.push_scene(std::make_shared<PlayScene>(game, std::string(zia::constants::LEVEL_PATHS.at(0))));
                        ImGui::EndMenu();
                        ImGui::EndMainMenuBar();
                        return;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Settings")) {
                    if (ImGui::MenuItem("Open Settings")) {
                        show_settings = true;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("View")) {
                    bool vis = zia::InspectorSystem::is_inspector_visible();
                    if (ImGui::MenuItem("Inspector", nullptr, vis)) {
                        zia::InspectorSystem::set_inspector_visible(!vis);
                    }
                    ImGui::EndMenu();
                }
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
            if (show_settings) {
                ImGui::Begin("Settings", &show_settings, ImGuiWindowFlags_AlwaysAutoResize);

                // Provide simple defaults mirroring MenuScene behavior. For brevity the settings values are
                // not directly bound to Game::settings() here; MenuScene handles Apply.
                const char* resolutions[] = { "800 x 600", "1024 x 768", "1280 x 720", "Fullscreen" };
                static int ui_resolution_index = 0;
                ImGui::Combo("Resolution", &ui_resolution_index, resolutions, IM_ARRAYSIZE(resolutions));
                static bool ui_fullscreen = false;
                ImGui::Checkbox("Fullscreen (override)", &ui_fullscreen);
                static float ui_master_volume = 1.0f;
                ImGui::SliderFloat("Master Volume", &ui_master_volume, 0.0f, 1.0f);

                if (ImGui::Button("Apply")) {
                    if (auto s = game.settings()) {
                        if (ui_resolution_index == 0) {
                            s->set_fullscreen(false);
                            s->set_window_size(800, 600);
                        } else if (ui_resolution_index == 1) {
                            s->set_fullscreen(false);
                            s->set_window_size(1024, 768);
                        } else if (ui_resolution_index == 2) {
                            s->set_fullscreen(false);
                            s->set_window_size(1280, 720);
                        } else if (ui_resolution_index == 3) {
                            s->set_fullscreen(true);
                        }
                        s->set_master_volume(ui_master_volume);
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Close")) {
                    show_settings = false;
                }

                ImGui::End();
            }
        }
    }
}
