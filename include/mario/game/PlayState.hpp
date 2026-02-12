#pragma once

#include "mario/game/GameStates.hpp"
#include "mario/systems/PhysicsSystems.hpp"
#include "mario/systems/RenderSystems.hpp"
#include "mario/systems/GameplaySystems.hpp"
#include "mario/world/World.hpp"
#include "mario/ui/HUD.hpp"
#include "mario/ecs/EntityManager.hpp"
#include "mario/ecs/Components.hpp"
#include "mario/helpers/Constants.hpp"
#include <string>

namespace mario {
    class Game;

    class PlayState : public GameState {
    public:
        PlayState(Game &game);

        PlayState(Game &game, std::string level_path);

        void on_enter() override;

        void on_exit() override;

        void update(float dt) override;

        void render() override;

        bool is_running() const override;

    private:
        void handle_level_transitions();

        void handle_input();

        void setup_systems();

        Game &_game;
        EntityID _player_id;
        Level _level;
        HUD _hud;
        bool _running = true;
        float _level_transition_delay = 0.0f;
        bool _level_transition_pending = false;
        std::string _current_level_path = std::string(mario::constants::LEVEL1_PATH);

        // Track the previous state of the ToggleDebug key to perform a rising-edge toggle
        bool _debug_toggle_last_state = false;
    };
} // namespace mario
