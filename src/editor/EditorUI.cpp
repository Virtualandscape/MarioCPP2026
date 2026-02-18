#include "Zia/editor/EditorUI.hpp"
#include "Zia/editor/EditorScene.hpp"

#include "Zia/engine/ecs/components/PositionComponent.hpp"
#include "Zia/engine/ecs/components/VelocityComponent.hpp"
#include "Zia/engine/ecs/components/SizeComponent.hpp"
#include "Zia/engine/ecs/components/TypeComponent.hpp"
#include "Zia/engine/ecs/EntityTypeComponent.hpp"
#include "Zia/engine/ecs/components/NameComponent.hpp"
#include "Zia/game/helpers/Constants.hpp"

#include <imgui.h>
#include <vector>
#include <cstdio>

namespace zia::editor {

// Static visibility flag for the editor UI.
static bool s_visible = false;

EditorUI::EditorUI(std::shared_ptr<zia::engine::IEntityManager> entities,
                   std::shared_ptr<zia::engine::IAssetManager> assets)
    : _entities(std::move(entities)), _assets(std::move(assets)) {
    // Create editor logic that uses the entity manager adapter.
    if (_entities && _assets) {
        _editor = create_editor_scene(*_entities, *_assets);
    }
}

void EditorUI::set_visible(bool v) { s_visible = v; }
bool EditorUI::visible() { return s_visible; }

void set_editor_visible(bool v) { EditorUI::set_visible(v); }
bool is_editor_visible() { return EditorUI::visible(); }

void EditorUI::build() {
    if (!s_visible) return;

    // Main editor window (pass pointer so the titlebar close button updates visibility)
    if (ImGui::Begin("Scene Editor", &s_visible)) {
        // Close on Escape key (if ImGui handled keyboard input)
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
            s_visible = false;
        }
        // Top controls
        // Use local buffer for InputText to safely edit std::string
        std::vector<char> buf(512);
        std::snprintf(buf.data(), buf.size(), "%s", _scene_path.c_str());
        if (ImGui::InputText("Scene Path", buf.data(), buf.size())) {
            _scene_path = std::string(buf.data());
        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            if (_editor) _editor->open_scene(_scene_path);
        }
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            if (_editor) _editor->save_scene(_scene_path);
        }

        ImGui::Separator();

        // Layout: left = hierarchy, center = viewport, right = inspector
        ImGui::Columns(3, "editor_cols");
        ImGui::SetColumnWidth(0, 250.0f);
        ImGui::SetColumnWidth(2, 300.0f);

        // Hierarchy
        draw_hierarchy();
        ImGui::NextColumn();

        // Viewport
        draw_viewport();
        ImGui::NextColumn();

        // Inspector
        draw_inspector();

        ImGui::Columns(1);
    }
    ImGui::End();
}

void EditorUI::draw_viewport() {
    ImGui::Text("Viewport");
    ImGui::BeginChild("ViewportPane", ImVec2(400, 300), true);
    ImGui::Text("(rendering not implemented yet)");
    ImGui::EndChild();
}

void EditorUI::draw_hierarchy() {
    ImGui::Text("Hierarchy");
    ImGui::BeginChild("HierarchyPane", ImVec2(0, 300), true);
    if (_entities) {
        std::vector<zia::EntityID> entities;
        _entities->get_entities_with<PositionComponent>(entities);
        for (auto id : entities) {
            // Prefer a name if available
            std::string label_str;
            if (auto name_c = _entities->get_component<NameComponent>(id)) {
                label_str = name_c->get().value + " (" + std::to_string(id) + ")";
            } else {
                label_str = std::string("Entity ") + std::to_string(id);
            }
            if (ImGui::Selectable(label_str.c_str(), _selected == id)) {
                _selected = id;
                if (_editor) _editor->select_entity(id);
            }
        }
    }
    ImGui::EndChild();
}

void EditorUI::draw_inspector() {
    ImGui::Text("Inspector");
    ImGui::BeginChild("InspectorPane", ImVec2(0, 300), true);
    if (_selected != 0 && _entities) {
        // Position component (pixels)
        if (auto p = _entities->get_component<PositionComponent>(_selected)) {
            float px = p->get().x;
            float py = p->get().y;
            if (ImGui::InputFloat("X (px)", &px)) {
                p->get().x = px;
            }
            if (ImGui::InputFloat("Y (px)", &py)) {
                p->get().y = py;
            }

            // Show and edit tile coordinates (integers). Editing tiles updates pixel pos.
            const int tile_size = zia::constants::TILE_SIZE;
            int tile_x = static_cast<int>(std::round(px / static_cast<float>(tile_size)));
            int tile_y = static_cast<int>(std::round(py / static_cast<float>(tile_size)));

            // If entity is player, compute tile_y differently (since player's Y uses feet offset)
            bool is_player = false;
            if (auto tc = _entities->get_component<TypeComponent>(_selected)) {
                if (tc->get().type == zia::EntityTypeComponent::Player) is_player = true;
            }

            int display_tile_y = tile_y;
            if (is_player) {
                // Convert player's world Y (feet aligned at world_y + (PLAYER_HEIGHT - tile_size)) to tile index
                display_tile_y = static_cast<int>(std::round((py + (zia::constants::PLAYER_HEIGHT - tile_size)) / static_cast<float>(tile_size)));
            }

            if (ImGui::InputInt("Tile X", &tile_x)) {
                // update pixel position
                p->get().x = static_cast<float>(tile_x * tile_size);
            }
            if (ImGui::InputInt("Tile Y", &display_tile_y)) {
                if (is_player) {
                    // player's world Y is tileY * tile_size - (PLAYER_HEIGHT - tile_size)
                    p->get().y = static_cast<float>(display_tile_y * tile_size - (zia::constants::PLAYER_HEIGHT - tile_size));
                } else {
                    p->get().y = static_cast<float>(display_tile_y * tile_size);
                }
            }
        }

        if (auto v = _entities->get_component<VelocityComponent>(_selected)) {
            float vx = v->get().vx;
            float vy = v->get().vy;
            if (ImGui::InputFloat("VX", &vx)) {
                v->get().vx = vx;
            }
            if (ImGui::InputFloat("VY", &vy)) {
                v->get().vy = vy;
            }
        }

        if (auto s = _entities->get_component<SizeComponent>(_selected)) {
            float w = s->get().width;
            float h = s->get().height;
            if (ImGui::InputFloat("W", &w)) {
                s->get().width = w;
            }
            if (ImGui::InputFloat("H", &h)) {
                s->get().height = h;
            }
        }
    } else {
        ImGui::TextUnformatted("No entity selected");
    }
    ImGui::EndChild();
}

} // namespace zia::editor

