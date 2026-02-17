#pragma once

#include <imgui.h>
#include <vector>
#include <string>
#include "mario/engine/IEntityManager.hpp"
#include "mario/engine/IAssetManager.hpp"

namespace mario {
    class InspectorSystem {
    public:
        InspectorSystem();

        // Update internal state (selection, scrolling) if needed
        void update(mario::engine::IEntityManager& registry, float dt);

        // Render using ImGui
        void render_ui(mario::engine::IEntityManager& registry, mario::engine::IAssetManager& assets);

        void set_enabled(bool en) { _enabled = en; }
        bool enabled() const { return _enabled; }
        void toggle_enabled() { _enabled = !_enabled; }

        // Configure how many entities to show to avoid overly large overlays
        void set_max_entries(size_t n) { _max_entries = n; }

    private:
        bool _enabled = true;
        size_t _max_entries = 32;

        // Helper to build the overlay lines
        void build_lines(const std::vector<EntityID>& entities, mario::engine::IEntityManager& registry, std::vector<std::string>& out_lines, mario::engine::IAssetManager& assets) const;
    };
}
