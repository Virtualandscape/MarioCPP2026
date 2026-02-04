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
        inline const sf::Color TILE_COLOR{138, 51, 36};

        // Enemy colors
        inline const sf::Color ENEMY_SPRITE_COLOR_BLACK{0, 0, 0};
        inline const sf::Color ENEMY_SPRITE_COLOR_RED{255, 0, 0};

        // Clouds
        static constexpr int CLOUD_BIG_ID = 2000;
        static constexpr int CLOUD_MEDIUM_ID = 2001;
        static constexpr int CLOUD_SMALL_ID = 2002;
        static constexpr int NUM_BIG_CLOUDS = 1;
        static constexpr int NUM_MEDIUM_CLOUDS = 2;
        static constexpr int NUM_SMALL_CLOUDS = 3;
        static constexpr float CLOUD_BIG_SPEED = 40.0f;
        static constexpr float CLOUD_MEDIUM_SPEED = 30.0f;
        static constexpr float CLOUD_SMALL_SPEED = 20.0f;
        static constexpr float CLOUD_RESET_X = 1100.0f;
        static constexpr float CLOUD_SPAWN_X = -300.0f;
        static constexpr float CLOUD_BIG_Y_MIN = 40.0f;
        static constexpr float CLOUD_BIG_Y_MAX = 60.0f;
        static constexpr float CLOUD_MEDIUM_Y_MIN = 60.0f;
        static constexpr float CLOUD_MEDIUM_Y_MAX = 80.0f;
        static constexpr float CLOUD_SMALL_Y_MIN = 80.0f;
        static constexpr float CLOUD_SMALL_Y_MAX = 100.0f;
        static constexpr float CLOUD_PARALLAX = 0.1f;
        static constexpr float CLOUD_SCALE = 1.0f;
        static constexpr float CLOUD_BIG_SPACING = 300.0f;
        static constexpr float CLOUD_MEDIUM_SPACING = 250.0f;
        static constexpr float CLOUD_SMALL_SPACING = 200.0f;
    } // namespace constants
} // namespace mario
