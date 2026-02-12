/*
#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <string>

#include "mario/ecs/components/PlayerControllerComponent.hpp"

namespace mario {

    // Identifies the type of an entity for logic and collisions.
    enum class EntityType {
        Unknown,
        Player,
        Goomba,
        Koopa
    };

    // Component that stores the entity type.
    struct TypeComponent {
        EntityType type = EntityType::Unknown;
    };

    // 2D position in the game world.
    struct PositionComponent {
        float x = 0.0f;
        float y = 0.0f;
    };

    // 2D velocity for physics and movement.
    struct VelocityComponent {
        float vx = 0.0f;
        float vy = 0.0f;
    };

    // Physical dimensions of the entity.
    struct SizeComponent {
        float width = 1.0f;
        float height = 1.0f;
    };

    // Rendering information for an entity.
    struct SpriteComponent {
        enum class Shape {
            Rectangle,
            Ellipse,
            None
        };

        Shape shape = Shape::Rectangle;
        sf::Color color = sf::Color::White;
        int texture_id = -1;
        // Offset applied when rendering the sprite relative to entity PositionComponent
        sf::Vector2f render_offset = {0.0f, 0.0f};
        // Optional render size: if both components > 0, use this for drawing instead of SizeComponent
        sf::Vector2f render_size = {0.0f, 0.0f};
        // Optional texture sub-rectangle (for animation frames)
        sf::IntRect texture_rect;
    };

    // Animation state for sprite-based entities.
    struct AnimationComponent {
        enum class State {
            Idle,
            Run,
            Jump
        };

        State current_state = State::Idle;
        int current_frame = 0;
        float frame_timer = 0.0f;
        float frame_duration = 0.1f;
        int frame_count = 1;
        bool flip_x = true;
        bool is_dirty = true; // To force initial rect update
    };

    // Parallax background layer configuration.
    struct BackgroundComponent {
        int texture_id = 0; // Key in AssetManager
        bool preserve_aspect = true;
        enum class ScaleMode { Fit, Fill } scale_mode = ScaleMode::Fit;
        // Multiplier applied to the computed scale (1.0 = no extra zoom). Use >1 to zoom in.
        float scale_multiplier = 1.0f;
        float parallax = 0.0f; // 0 = follows camera, 1 = fixed to world
        float offset_x = 0.0f; // Offset along the X axis
        float offset_y = 0.0f; // Offset along the Y axis
        bool repeat = true; // Repeat both horizontally and vertically
        bool repeat_x = false; // Repeat only horizontally (horizontal)
    };

    // Configuration for decorative cloud entities.
    struct CloudComponent {
        int texture_id = 0; // Key in AssetManager
        enum class Layer { Big, Medium, Small } layer = Layer::Big;
        float speed = 10.0f; // Pixels per second
        float x = 0.0f; // Position x
        float y = 0.0f; // Position y
        float scale = 1.0f; // Scale multiplier
    };

    // Collision metadata for reaction logic.
    struct CollisionInfoComponent {
        bool collided = false;
        EntityType other_type = EntityType::Unknown;
    };

    // Data for enemy AI and state.
    struct EnemyComponent {
        bool active = true;
    };

    // PlayerControllerComponent is provided via dedicated header and contains input and movement state.

} // namespace mario
*/

