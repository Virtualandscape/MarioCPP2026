#pragma once

#include "Zia/engine/IScene.hpp"
#include "Zia/engine/Scene.hpp"

#include <memory>

namespace zia::engine::adapters {
    // Adapter that wraps a game-specific Zia::Scene and exposes it as an engine::IScene.
    class SceneAdapter : public IScene {
    public:
        explicit SceneAdapter(std::shared_ptr<zia::Scene> scene) : _scene(std::move(scene)) {}
        ~SceneAdapter() override = default;

        void on_enter() override { if (_scene) _scene->on_enter(); }
        void on_exit() override { if (_scene) _scene->on_exit(); }
        void update(float dt) override { if (_scene) _scene->update(dt); }
        void render() override { if (_scene) _scene->render(); }
        [[nodiscard]] bool is_running() const override { return _scene ? _scene->is_running() : false; }

        // Return the underlying Zia::Scene shared_ptr for callers that need the concrete type.
        [[nodiscard]] std::shared_ptr<zia::Scene> underlying() const { return _scene; }

    private:
        std::shared_ptr<zia::Scene> _scene;
    };
} // namespace Zia::engine::adapters

