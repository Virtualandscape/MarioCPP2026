#include "../../../include/Zia/engine/ui/UIManager.hpp"

#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>

namespace zia::engine {
    bool UIManager::init(sf::RenderWindow &window) {
        if (!ImGui::SFML::Init(window)) {
            std::cerr << "ImGui-SFML initialization failed" << std::endl;
            return false;
        }
        return true;
    }

    void UIManager::shutdown() {
        ImGui::SFML::Shutdown();
    }

    void UIManager::process_event(sf::RenderWindow &window, const sf::Event &event) {
        ImGui::SFML::ProcessEvent(window, event);
    }

    void UIManager::update(sf::RenderWindow &window, sf::Clock &clock) {
        ImGui::SFML::Update(window, clock.restart());
        // Measure a reasonable default menu bar height from ImGui metrics. This uses the standard frame height
        // which corresponds to the height of typical widgets (buttons) and works well for the main menu bar.
        if (ImGui::GetCurrentContext()) {
            // Add a small padding to ensure the menu does not overlap content
            const float fh = ImGui::GetFrameHeight();
            _menu_bar_height = static_cast<int>(fh + 4.0f);
        }
    }

    void UIManager::render(sf::RenderWindow &window) {
        // Ensure UI is rendered in screen-space: use the default view temporarily.
        auto old_view = window.getView();
        window.setView(window.getDefaultView());
        ImGui::SFML::Render(window);
        window.setView(old_view);
    }
} // namespace Zia::engine
