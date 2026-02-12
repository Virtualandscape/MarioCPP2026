// Connects the platformer Game implementation to the reusable engine infrastructure.

#include "mario/game/Game.hpp"
#include "mario/game/GameStates.hpp"

#include <memory>

namespace mario {

Game::Game() = default;

// Ensure the platformer starts by pushing the main menu if no state is present.
void Game::before_loop() {
    if (!current_state()) {
        push_state(std::make_shared<MenuState>(*this));
    }
}

} // namespace mario
