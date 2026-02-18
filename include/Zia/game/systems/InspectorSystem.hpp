#pragma once

#include <imgui.h>
#include <vector>
#include <string>
#include "Zia/engine/IEntityManager.hpp"
#include "Zia/engine/IAssetManager.hpp"

namespace zia {
    class InspectorSystem {
    public:
        InspectorSystem();

        // Update internal state (selection, scrolling) if needed
        void update(zia::engine::IEntityManager& registry, float dt);

        // Render using ImGui
        void render_ui(zia::engine::IEntityManager& registry, zia::engine::IAssetManager& assets);

        void set_enabled(bool en) { _enabled = en; }
        bool enabled() const { return _enabled; }
        void toggle_enabled() { _enabled = !_enabled; }

        // Configure how many entities to show to avoid overly large overlays
        void set_max_entries(size_t n) { _max_entries = n; }

        // Global visibility helpers (menu can call these to toggle inspector across scenes)
        static void set_inspector_visible(bool v);
        static bool is_inspector_visible();

    private:
        bool _enabled = true;
        size_t _max_entries = 32;

        // Helper to build the overlay lines
        void build_lines(const std::vector<EntityID>& entities, zia::engine::IEntityManager& registry, std::vector<std::string>& out_lines, zia::engine::IAssetManager& assets) const;
    };
}
