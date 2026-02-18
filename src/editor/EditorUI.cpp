#include "Zia/editor/EditorUI.hpp"
#include "Zia/editor/EditorScene.hpp"

#include "Zia/engine/ecs/components/PositionComponent.hpp"
#include "Zia/engine/ecs/components/VelocityComponent.hpp"
#include "Zia/engine/ecs/components/SizeComponent.hpp"

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
    if (_entities) {
        _editor = create_editor_scene(*_entities);
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
            char label[32];
            std::snprintf(label, sizeof(label), "Entity %u", id);
            if (ImGui::Selectable(label, _selected == id)) {
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
        if (auto p = _entities->get_component<PositionComponent>(_selected)) {
            float x = p->get().x;
            float y = p->get().y;
            if (ImGui::InputFloat("X", &x)) {
                p->get().x = x;
            }
            if (ImGui::InputFloat("Y", &y)) {
                p->get().y = y;
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

