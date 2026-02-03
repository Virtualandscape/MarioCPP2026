#pragma once

#include <SFML/Graphics/Color.hpp>

namespace mario {
namespace constants {

// Player
static constexpr float PLAYER_WIDTH = 16.0f;
static constexpr float PLAYER_HEIGHT = 24.0f;
static constexpr float PLAYER_DEFAULT_X = 32.0f;
static constexpr float PLAYER_DEFAULT_Y = 32.0f;

// Velocities
static constexpr float ZERO_VELOCITY = 0.0f;

// Enemy
static constexpr float ENEMY_INITIAL_SPEED = -30.0f;
static constexpr float ENEMY_SIZE = 16.0f;

// Colors
inline const sf::Color PLAYER_SPRITE_COLOR_GREEN{50, 100, 80};
inline const sf::Color PLAYER_SPRITE_COLOR_RED{200, 0, 0};

// Enemy colors
inline const sf::Color ENEMY_SPRITE_COLOR_BLACK{0, 0, 0};
inline const sf::Color ENEMY_SPRITE_COLOR_RED{255, 0, 0};

} // namespace constants
} // namespace mario
