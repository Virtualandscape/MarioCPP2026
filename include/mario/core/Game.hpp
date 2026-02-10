#pragma once

#include "mario/engine/Engine.hpp"

namespace mario {

// Platformer-specific game that wires the engine to the menu state.
class Game : public engine::Engine {
public:
    Game();

protected:
    void before_loop() override;
};

} // namespace mario
