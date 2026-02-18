#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>

namespace zia::engine {
    // Lightweight wrapper around ImGui-SFML lifecycle (Init/Shutdown/ProcessEvent/Update/Render).
    // Keeps ImGui usage centralized and easier to debug when flicker or ordering issues occur.
    class UIManager {
    public:
        UIManager() = default;
        virtual ~UIManager() = default;

        // Initialize ImGui with the provided RenderWindow. Returns true on success.
        bool init(sf::RenderWindow& window);

        // Shutdown ImGui and release resources.
        void shutdown();

        // Forward an SFML event to ImGui for processing.
        void process_event(sf::RenderWindow& window, const sf::Event& event);

        // Update ImGui state. Accepts an sf::Clock reference for proper delta.
        void update(sf::RenderWindow& window, sf::Clock& clock);

        // Build ImGui widgets for this frame (called after update and before render).
        // Default implementation is empty; games can override this or provide a callback.
        virtual void build() {}

        // Render ImGui draw data on top of the current frame; call after scene rendering.
        void render(sf::RenderWindow& window);

        // Menu bar height helpers: UI can report measured menu bar height in pixels
        // so other subsystems can reserve UI space at the top of the window.
        void set_menu_bar_height(int px) { _menu_bar_height = px; }
        int menu_bar_height() const { return _menu_bar_height; }

    private:
        int _menu_bar_height = 0;
    };
} // namespace Zia::engine

