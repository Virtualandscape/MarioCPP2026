#pragma once

#include "Entity.hpp"
#include "mario/world/TileMap.hpp"

namespace mario {
    // Input handling, jump/run state, power-ups.
    class Player : public Entity {
    public:
        Player();

        void handle_input();

        void set_move_axis(float axis);

        void set_jump_pressed(bool pressed);

        bool is_on_ground(const TileMap& map) const;

        bool is_jumping() const { return _jump_count > 0; }

        void reset_jump();

        void update(float dt) override;

        void render(Renderer& renderer)  override;
        
        EntityTypeComponent type() const override { return EntityTypeComponent::Player; }

    private:
        int _jump_count = 0;
        float _move_axis = 0.0f;
        bool _jump_pressed = false;
        bool _jump_held = false;
        float _move_speed = 220.0f;
        float _jump_speed = 400.0f;
    };
} // namespace mario
