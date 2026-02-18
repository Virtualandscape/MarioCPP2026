#pragma once

#include <imgui.h>
#include "Zia/game/MarioGame.hpp"

namespace zia {
    namespace ui {
        // Draw the application's top main menu bar. The function uses ImGui and may be called
        // every frame while an ImGui frame is active. The caller must ensure ImGui::SFML::Update
        // has already been called for the current frame. show_settings is a shared flag used to
        // toggle the Settings window; passing by reference allows MenuScene and the overlay to share state.
        void draw_main_menu_bar(zia::Game &game, bool &show_settings);
    }
}

