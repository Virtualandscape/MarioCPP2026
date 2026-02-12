#pragma once

#include "mario/ecs/EntityManager.hpp"
#include "mario/input/InputManager.hpp"
#include <string>

namespace mario {
    class TileMap;
    class Level;
}

namespace mario::systems::gameplay {
    // Processes user input and updates the PlayerInputComponent.
    void handle_player_input(EntityManager& registry, const InputManager& input);

    // Updates player movement (jumping, horizontal speed).
    void update_player_movement(EntityManager& registry, float dt);

    // Updates enemy AI and simple movement.
    void update_enemies(EntityManager& registry, const TileMap& map, float dt);

    // Monitors level progression and transitions.
    bool check_level_transitions(EntityManager& registry, EntityID player_id, Level& level,
                                 std::string& current_level_path, float& transition_delay, float dt);
}

