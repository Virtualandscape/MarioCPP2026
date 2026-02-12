#pragma once

#include "mario/engine/Engine.hpp"

namespace mario {

    // Main game class specializing the engine for a platformer.
    class Game : public engine::Engine {
    public:
        Game();

    protected:
        void before_loop() override;
    };

} // namespace mario

