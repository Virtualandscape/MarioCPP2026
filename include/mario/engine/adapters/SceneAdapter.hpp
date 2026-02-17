#pragma once

#include "mario/engine/IScene.hpp"
#include "mario/engine/Scene.hpp"

#include <memory>

namespace mario::engine::adapters {
    // Adapter that wraps a game-specific mario::Scene and exposes it as an engine::IScene.
    class SceneAdapter : public IScene {
    public:
        explicit SceneAdapter(std::shared_ptr<mario::Scene> scene) : _scene(std::move(scene)) {}
        ~SceneAdapter() override = default;

        void on_enter() override { if (_scene) _scene->on_enter(); }
        void on_exit() override { if (_scene) _scene->on_exit(); }
        void update(float dt) override { if (_scene) _scene->update(dt); }
        void render() override { if (_scene) _scene->render(); }
        [[nodiscard]] bool is_running() const override { return _scene ? _scene->is_running() : false; }

        // Return the underlying mario::Scene shared_ptr for callers that need the concrete type.
        [[nodiscard]] std::shared_ptr<mario::Scene> underlying() const { return _scene; }

    private:
        std::shared_ptr<mario::Scene> _scene;
    };
} // namespace mario::engine::adapters

