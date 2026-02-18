#pragma once

#include "Zia/engine/ui/UIManager.hpp"
#include "Zia/editor/ISceneEditor.hpp"

#include <memory>
#include <string>

namespace zia::engine { class IEntityManager; class IAssetManager; }

namespace zia::editor {

// Simple Editor UI using ImGui. Shows Viewport, Hierarchy and Inspector panels.
class EditorUI : public zia::engine::UIManager {
public:
    // Construct with interfaces to entity and asset managers.
    EditorUI(std::shared_ptr<zia::engine::IEntityManager> entities,
             std::shared_ptr<zia::engine::IAssetManager> assets);
    ~EditorUI() override = default;

    // Build ImGui widgets for the editor.
    void build() override;

    // Visibility control (menu toggles these functions)
    static void set_visible(bool v);
    static bool visible();

private:
    // Panel helpers
    void draw_viewport();
    void draw_hierarchy();
    void draw_inspector();

    std::shared_ptr<zia::engine::IEntityManager> _entities;
    std::shared_ptr<zia::engine::IAssetManager> _assets;
    std::unique_ptr<ISceneEditor> _editor; // owns the editor logic

    // Local editor state
    std::string _scene_path = "assets/levels/level1.json";
    zia::EntityID _selected = 0;
};

// Simple visibility helpers usable from other modules (MenuScene)
void set_editor_visible(bool v);
bool is_editor_visible();

} // namespace zia::editor

