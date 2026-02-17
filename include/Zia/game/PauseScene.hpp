#pragma once

#include "Zia/engine/Scene.hpp"

namespace zia {
    // Scene representing a paused game; no-op implementations by default.
    class PauseScene : public Scene {
    public:
        void on_enter() override;
        void on_exit() override;
        void update(float dt) override;
        void render() override;
    };
} // namespace Zia

