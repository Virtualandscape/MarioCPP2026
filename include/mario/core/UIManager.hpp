#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>

namespace mario {
    // Lightweight wrapper around ImGui-SFML lifecycle (Init/Shutdown/ProcessEvent/Update/Render).
    // Keeps ImGui usage centralized and easier to debug when flicker or ordering issues occur.
    class UIManager {
    public:
        UIManager() = default;
        ~UIManager() = default;

        // Initialize ImGui with the provided RenderWindow. Returns true on success.
        bool init(sf::RenderWindow& window);

        // Shutdown ImGui and release resources.
        void shutdown();

        // Forward an SFML event to ImGui for processing.
        void process_event(sf::RenderWindow& window, const sf::Event& event);

        // Update ImGui state. Accepts an sf::Clock reference for proper delta.
        void update(sf::RenderWindow& window, sf::Clock& clock);

        // Build ImGui widgets for this frame (called after update and before render).
        void build();

        // Render ImGui draw data on top of the current frame; call after scene rendering.
        void render(sf::RenderWindow& window);
    };
} // namespace mario

