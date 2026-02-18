// Connects the platformer Game implementation to the reusable engine infrastructure.

#include "Zia/game/MarioGame.hpp"
#include "Zia/game/MenuScene.hpp"
#include "Zia/engine/adapters/SceneAdapter.hpp"
#include "Zia/engine/EngineConfig.hpp"
#include "Zia/engine/audio/AudioManager.hpp"
// Include shared main menu bar utility so we can register it as a global overlay.
#include "Zia/game/ui/MainMenuBar.hpp"


#include <memory>

// ImGui is accessed via UIManager to centralize lifecycle and rendering.

namespace zia {
    // Construct: create the underlying engine application which owns subsystems.
    Game::Game()
        : _app(std::make_unique<engine::Application>("Zia")),
          _settings(std::make_shared<engine::EngineConfig>())
    {
        // Register observer to apply runtime-visible changes (resize, volume)
        // Keep observer id if needed later (not currently unregistered; acceptable for app lifetime)
        _settings->register_observer([this](const engine::EngineConfig &cfg) {
            // Apply window size change on renderer
            try {
                _app->renderer().window().setSize(sf::Vector2u(static_cast<unsigned int>(cfg.window_width()), static_cast<unsigned int>(cfg.window_height())));
            } catch (...) {
                // Renderer may not expose setSize; ignore errors and leave as best-effort.
            }
            // Apply master volume if audio manager exists in application (best-effort)
            try {
                // The engine's AudioManager currently lives in src/engine/audio; call set_volume globally if accessible.
                // We don't have a global accessor here, so attempt to call through assets or other path isn't available; fallback: no-op
                zia::AudioManager audio; // local no-op; real audio manager integration is future work
                audio.set_volume(cfg.master_volume());
            } catch (...) {
            }
        });

        // Register overlay using the MainMenuBar utility (namespaced in zia::ui)
        _app->set_ui_overlay([this]() {
            zia::ui::draw_main_menu_bar(*this, _menu_show_settings);
        });
    }

    // Default destructor: unique_ptr will clean up the engine application.
    Game::~Game() = default;

    // Initialize runtime via the engine application.
    void Game::initialize() {
        _app->initialize();
    }

    // Shutdown is forwarded to the engine application.
    void Game::shutdown() {
        _app->shutdown();
    }

    // Run the full lifecycle through the engine application.
    void Game::run() {
        // Allow Game to perform any game-specific setup before run if needed.
        before_loop();
        _app->run();
    }

    // Forward scene management to the engine application.
    void Game::push_scene(std::shared_ptr<Scene> scene) {
        // Wrap the game-specific Scene into the engine adapter before pushing.
        _app->push_scene(std::make_shared<engine::adapters::SceneAdapter>(std::move(scene)));
    }

    void Game::pop_scene() {
        _app->pop_scene();
    }

    std::shared_ptr<Scene> Game::current_scene() {
        auto iscene = _app->current_scene();
        if (!iscene) return nullptr;
        // Try to cast to SceneAdapter to obtain the underlying concrete Zia::Scene.
        if (auto adapter = std::dynamic_pointer_cast<engine::adapters::SceneAdapter>(iscene)) {
            return adapter->underlying();
        }
        // Not an adapter-wrapped scene: cannot provide a concrete Zia::Scene pointer.
        return nullptr;
    }

    zia::engine::IRenderer &Game::renderer() {
        return _app->renderer();
    }

    zia::engine::IInput &Game::input() {
        return _app->input();
    }

    zia::engine::IAssetManager &Game::assets() {
        return _app->assets();
    }

    // Provide both facade and, for compatibility, access to the underlying concrete EntityManager.
    zia::engine::IEntityManager &Game::entity_manager() {
        return _app->entity_manager();
    }

    zia::EntityManager &Game::underlying_entity_manager() {
        return _app->underlying_entity_manager();
    }

    // Expose settings manager
    std::shared_ptr<zia::engine::EngineConfig> Game::settings() {
        return _settings;
    }

    // Hook: if no scene is present, maintain previous behavior and push MenuScene.
    void Game::before_loop() {
        if (!_app->current_scene()) {
            push_scene(std::make_shared<MenuScene>(*this));
        }
    }
} // namespace Zia
