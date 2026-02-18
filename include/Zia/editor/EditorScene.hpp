#pragma once

#include "Zia/editor/ISceneEditor.hpp"
#include <memory>

namespace zia::engine { class IEntityManager; class IAssetManager; }

namespace zia::editor {

// Concrete editor scene implementation declaration.
class EditorScene : public ISceneEditor {
public:
    // Construct with references to the engine's IEntityManager and IAssetManager adapters.
    EditorScene(zia::engine::IEntityManager& mgr, zia::engine::IAssetManager& assets);
    ~EditorScene() override = default;

    // ISceneEditor implementation
    bool open_scene(const std::string& path) override;
    bool save_scene(const std::string& path) override;
    void play() override;
    void pause() override;
    zia::EntityID spawn_entity() override;
    void select_entity(zia::EntityID id) override;
    void undo() override;
    void redo() override;

private:
    // References to the engine-level adapters. Not owned by the editor.
    zia::engine::IEntityManager& _mgr;
    zia::engine::IAssetManager& _assets;
    // Currently selected entity id in editor (0 = none).
    zia::EntityID _selected = 0;
};

// Factory to create an EditorScene instance returning the interface type.
std::unique_ptr<ISceneEditor> create_editor_scene(zia::engine::IEntityManager& mgr, zia::engine::IAssetManager& assets);

} // namespace zia::editor

