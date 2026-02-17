#pragma once

#include "mario/render/Renderer.hpp"
#include "mario/ecs/EntityManager.hpp"
#include "mario/world/Camera.hpp"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <optional>
#include <string>
#include <vector>
#include <memory>

namespace mario {
    class AssetManager; // forward declare to avoid including heavy header in this small system header

    class InspectorSystem {
    public:
        InspectorSystem();

        // Load assets (font) if needed
        void initialize(AssetManager& assets);

        // Update internal state (selection, scrolling) if needed
        void update(EntityManager& registry, float dt);

        // Render overlay: draw inspected components for player and enemies
        void render(Renderer& renderer, const Camera& camera, EntityManager& registry, AssetManager& assets);

        void set_enabled(bool en) { _enabled = en; }
        bool enabled() const { return _enabled; }
        void toggle_enabled() { _enabled = !_enabled; }

        // Configure how many entities to show to avoid overly large overlays
        void set_max_entries(size_t n) { _max_entries = n; }

    private:
        bool _enabled = true;
        size_t _max_entries = 32;

        // Fallback font used to construct sf::Text; prefer AssetManager-provided fonts via _font_ptr.
        sf::Font _fallback_font;
        sf::Text _text;
        std::shared_ptr<const sf::Font> _font_ptr;

        // Helper to build the overlay lines
        void build_lines(const std::vector<EntityID>& entities, EntityManager& registry, std::vector<std::string>& out_lines, AssetManager& assets) const;
    };
}
