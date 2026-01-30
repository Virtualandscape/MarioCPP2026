#include "mario/core/GameState.hpp"

#include <algorithm>
#include <cmath>

namespace mario {
    namespace {
        struct Aabb {
            float left;
            float top;
            float right;
            float bottom;
        };

        Aabb make_aabb(float x, float y, float w, float h)
        {
            return {x, y, x + w, y + h};
        }

        bool intersects(const Aabb& a, const Aabb& b)
        {
            return a.left < b.right && a.right > b.left && a.top < b.bottom && a.bottom > b.top;
        }

        void resolve_player_vs_world(Player& player, const TileMap& map, float dt)
        {
            const float w = player.width();
            const float h = player.height();
            const float current_x = player.x();
            const float current_y = player.y();
            const float prev_x = current_x - player.vx() * dt;
            const float prev_y = current_y - player.vy() * dt;
            const int tile_size = map.tile_size();

            if (tile_size <= 0) {
                return;
            }

            float new_x = current_x;
            float new_y = current_y;
            float vx = player.vx();
            float vy = player.vy();

            if (vx != 0.0f) {
                const float min_x = std::min(prev_x, current_x);
                const float max_x = std::max(prev_x, current_x);
                const int start_x = static_cast<int>(std::floor(min_x / tile_size));
                const int end_x = static_cast<int>(std::floor((max_x + w) / tile_size));
                const int start_y = static_cast<int>(std::floor(prev_y / tile_size));
                const int end_y = static_cast<int>(std::floor((prev_y + h) / tile_size));

                for (int ty = start_y; ty <= end_y; ++ty) {
                    for (int tx = start_x; tx <= end_x; ++tx) {
                        if (!map.is_solid(tx, ty)) {
                            continue;
                        }

                        const float tile_left = static_cast<float>(tx * tile_size);
                        const float tile_top = static_cast<float>(ty * tile_size);
                        const Aabb tile_box = {tile_left, tile_top, tile_left + tile_size, tile_top + tile_size};
                        const Aabb player_box = make_aabb(new_x, prev_y, w, h);
                        if (!intersects(player_box, tile_box)) {
                            continue;
                        }

                        if (vx > 0.0f) {
                            new_x = tile_left - w;
                        } else {
                            new_x = tile_left + tile_size;
                        }
                        vx = 0.0f;
                    }
                }
            }

            if (vy != 0.0f) {
                const float min_y = std::min(prev_y, current_y);
                const float max_y = std::max(prev_y, current_y);
                const int start_x = static_cast<int>(std::floor(new_x / tile_size));
                const int end_x = static_cast<int>(std::floor((new_x + w) / tile_size));
                const int start_y = static_cast<int>(std::floor(min_y / tile_size));
                const int end_y = static_cast<int>(std::floor((max_y + h) / tile_size));

                for (int ty = start_y; ty <= end_y; ++ty) {
                    for (int tx = start_x; tx <= end_x; ++tx) {
                        if (!map.is_solid(tx, ty)) {
                            continue;
                        }

                        const float tile_left = static_cast<float>(tx * tile_size);
                        const float tile_top = static_cast<float>(ty * tile_size);
                        const Aabb tile_box = {tile_left, tile_top, tile_left + tile_size, tile_top + tile_size};
                        const Aabb player_box = make_aabb(new_x, new_y, w, h);
                        if (!intersects(player_box, tile_box)) {
                            continue;
                        }

                        if (vy > 0.0f) {
                            new_y = tile_top - h;
                        } else {
                            new_y = tile_top + tile_size;
                        }
                        vy = 0.0f;
                    }
                }
            }

            player.set_position(new_x, new_y);
            player.set_velocity(vx, vy);
        }
    }

    void PlayState::on_enter() {
        _tile_map.load("assets/levels/demo.json");
        _player.set_position(32.0f, 32.0f);
    }

    void PlayState::on_exit() {
    }

    void PlayState::update(float dt) {
        _input.poll();

        float axis = 0.0f;
        if (_input.is_pressed(InputManager::Action::MoveLeft)) {
            axis -= 1.0f;
        }
        if (_input.is_pressed(InputManager::Action::MoveRight)) {
            axis += 1.0f;
        }

        _player.set_move_axis(axis);
        _player.set_jump_pressed(_input.is_pressed(InputManager::Action::Jump));
        _player.handle_input();
        _player.update(dt);
        resolve_player_vs_world(_player, _tile_map, dt);
    }

    void PlayState::render() {
        _renderer.begin_frame();
        const int tile_size = _tile_map.tile_size();
        for (int ty = 0; ty < _tile_map.height(); ++ty) {
            for (int tx = 0; tx < _tile_map.width(); ++tx) {
                if (_tile_map.is_solid(tx, ty)) {
                    _renderer.draw_rect(
                        static_cast<float>(tx * tile_size),
                        static_cast<float>(ty * tile_size),
                        static_cast<float>(tile_size),
                        static_cast<float>(tile_size));
                }
            }
        }

        _renderer.draw_ellipse(_player.x(), _player.y(), _player.width(), _player.height());
        _renderer.end_frame();
    }

    bool PlayState::is_running() const { return _renderer.is_open(); }

    void MenuState::on_enter() {
    }

    void MenuState::on_exit() {
    }

    void MenuState::update(float dt) { (void) dt; }

    void MenuState::render() {
    }

    void PauseState::on_enter() {
    }

    void PauseState::on_exit() {
    }

    void PauseState::update(float dt) { (void) dt; }

    void PauseState::render() {
    }
} // namespace mario
