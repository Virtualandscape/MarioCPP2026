#pragma once

#include <string>
#include <cstdint>

#include "Zia/engine/IEntityManager.hpp"

namespace zia::editor {

// Interface for the integrated scene editor.
// Provides basic operations required by the UI and application.
struct ISceneEditor {
    virtual ~ISceneEditor() = default;

    // Open an existing scene file. Returns true on success.
    virtual bool open_scene(const std::string& path) = 0;

    // Save the current scene to disk. Returns true on success.
    virtual bool save_scene(const std::string& path) = 0;

    // Start simulation (play-in-editor).
    virtual void play() = 0;

    // Pause simulation and return to edit mode.
    virtual void pause() = 0;

    // Spawn a new empty entity and return its id.
    virtual zia::EntityID spawn_entity() = 0;

    // Select an entity in the editor UI.
    virtual void select_entity(zia::EntityID id) = 0;

    // Basic undo/redo commands.
    virtual void undo() = 0;
    virtual void redo() = 0;
};

} // namespace zia::editor

