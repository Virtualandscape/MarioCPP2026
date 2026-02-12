#include "mario/systems/GameplaySystems.hpp"
#include "mario/ecs/Components.hpp"
#include "mario/world/World.hpp"
#include "mario/helpers/Constants.hpp"
#include <vector>

namespace mario::systems::gameplay {

    void handle_player_input(EntityManager& registry, const InputManager& input) {
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<PlayerInputComponent>(entities);

        const bool jump = input.is_pressed(InputManager::Action::Jump);
        const float axis = (input.is_pressed(InputManager::Action::MoveLeft) ? -1.0f : 0.0f) +
                           (input.is_pressed(InputManager::Action::MoveRight) ? 1.0f : 0.0f);

        for (auto entity : entities) {
            auto input_opt = registry.get_component<PlayerInputComponent>(entity);
            if (input_opt) {
                auto& pi = input_opt->get();
                pi.jump_held = pi.jump_pressed;
                pi.jump_pressed = jump;
                pi.move_axis = axis;
            }
        }
    }

    void update_player_movement(EntityManager& registry, float dt) {
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<PlayerInputComponent, VelocityComponent>(entities);

        for (auto entity : entities) {
            auto input_opt = registry.get_component<PlayerInputComponent>(entity);
            auto vel_opt = registry.get_component<VelocityComponent>(entity);
            if (input_opt && vel_opt) {
                auto& input = input_opt->get();
                auto& vel = vel_opt->get();

                vel.vx = input.move_axis * constants::PLAYER_MOVE_SPEED;

                if (input.jump_pressed && !input.jump_held && input.jump_count < 2) {
                    vel.vy = -constants::jump_speed_for_tiles(constants::PLAYER_JUMP_TILES);
                    input.jump_count++;
                }
            }
        }
    }

    void update_enemies(EntityManager& registry, const TileMap& map, float dt) {
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with_all<EnemyComponent, VelocityComponent, PositionComponent, SizeComponent>(entities);

        for (auto entity : entities) {
            auto vel_opt = registry.get_component<VelocityComponent>(entity);
            auto pos_opt = registry.get_component<PositionComponent>(entity);
            auto size_opt = registry.get_component<SizeComponent>(entity);
            auto enemy_opt = registry.get_component<EnemyComponent>(entity);

            if (vel_opt && pos_opt && size_opt && enemy_opt) {
                auto& vel = vel_opt->get();
                auto& pos = pos_opt->get();
                auto& size = size_opt->get();

                // Simple patrol: reverse on collision with solid tile or ledge.
                const auto ts = static_cast<float>(map.tile_size());
                float next_x = pos.x + (vel.vx > 0 ? size.width : 0) + vel.vx * dt;
                int tx = static_cast<int>(next_x / ts);
                int ty = static_cast<int>((pos.y + size.height * 0.5f) / ts);

                if (map.is_solid(tx, ty)) {
                    vel.vx = -vel.vx;
                } else {
                    // Ledge detection
                    int foot_tx = static_cast<int>((pos.x + (vel.vx > 0 ? size.width : 0)) / ts);
                    int foot_ty = static_cast<int>((pos.y + size.height + 1.0f) / ts);
                    if (!map.is_solid(foot_tx, foot_ty)) {
                        vel.vx = -vel.vx;
                    }
                }
            }
        }
    }

    bool check_level_transitions(EntityManager& registry, EntityID player_id, Level& level,
                                 std::string& current_level_path, float& transition_delay, float dt) {
        if (player_id == 0) return false;

        auto pos_opt = registry.get_component<PositionComponent>(player_id);
        if (!pos_opt) return false;

        // Fall death check
        if (pos_opt->get().y > 1000.0f) {
            transition_delay -= dt;
            if (transition_delay <= 0) return true;
        }

        return false;
    }
}
