#pragma once

#include <SFML/Graphics/Color.hpp>
#include <array>


    namespace mario::constants {
        // Player
        inline constexpr float PLAYER_WIDTH = 64.0f;
        inline constexpr float PLAYER_HEIGHT = 64.0f;
        inline constexpr float PLAYER_DEFAULT_X = 32.0f;
        inline constexpr float PLAYER_DEFAULT_Y = 32.0f;

        // Velocities
        inline constexpr float ZERO_VELOCITY = 0.0f;
        // Player movement defaults (used as global defaults when no per-entity component is present)
        inline constexpr float PLAYER_MOVE_SPEED = 220.0f;
        inline constexpr float PLAYER_JUMP_SPEED = 400.0f;

        // Enemy
        inline constexpr float ENEMY_INITIAL_SPEED = -30.0f;
        inline constexpr float ENEMY_SIZE = 16.0f;

        // Colors
        inline const sf::Color PLAYER_SPRITE_COLOR_GREEN{50, 100, 80};
        inline const sf::Color PLAYER_SPRITE_COLOR_RED{200, 0, 0};
        inline const sf::Color TILE_COLOR{138, 51, 36};

        // Enemy colors
        inline const sf::Color ENEMY_SPRITE_COLOR_BLACK{0, 0, 0};
        inline const sf::Color ENEMY_SPRITE_COLOR_RED{255, 0, 0};

        // Background texture id used as base id for level backgrounds.
        // Keep as constexpr so it can be used in compile-time contexts.
        inline constexpr int BACKGROUND_TEXTURE_ID = 1000;

        // Clouds
        inline constexpr int CLOUD_BIG_ID = 2000;
        inline constexpr int CLOUD_MEDIUM_ID = 2001;
        inline constexpr int CLOUD_SMALL_ID = 2002;
        inline constexpr int NUM_BIG_CLOUDS = 1;
        inline constexpr int NUM_MEDIUM_CLOUDS = 2;
        inline constexpr int NUM_SMALL_CLOUDS = 3;
        inline constexpr float CLOUD_BIG_SPEED = 40.0f;
        inline constexpr float CLOUD_MEDIUM_SPEED = 30.0f;
        inline constexpr float CLOUD_SMALL_SPEED = 20.0f;
        inline constexpr float CLOUD_RESET_X = 1100.0f;
        inline constexpr float CLOUD_SPAWN_X = -300.0f;
        inline constexpr float CLOUD_BIG_Y_MIN = 40.0f;
        inline constexpr float CLOUD_BIG_Y_MAX = 60.0f;
        inline constexpr float CLOUD_MEDIUM_Y_MIN = 60.0f;
        inline constexpr float CLOUD_MEDIUM_Y_MAX = 80.0f;
        inline constexpr float CLOUD_SMALL_Y_MIN = 80.0f;
        inline constexpr float CLOUD_SMALL_Y_MAX = 100.0f;
        inline constexpr float CLOUD_PARALLAX = 0.1f;
        inline constexpr float CLOUD_SCALE = 1.0f;
        inline constexpr float CLOUD_BIG_SPACING = 300.0f;
        inline constexpr float CLOUD_MEDIUM_SPACING = 250.0f;
        inline constexpr float CLOUD_SMALL_SPACING = 200.0f;
        // Level paths
        inline constexpr const char* LEVEL1_PATH = "assets/levels/level1.json";
        inline constexpr const char* LEVEL2_PATH = "assets/levels/level2.json";

        // Central list of available levels (single definition point)
        inline constexpr std::array<const char*, 2> LEVEL_PATHS = { LEVEL1_PATH, LEVEL2_PATH };

        // Player Textures
        inline constexpr int PLAYER_IDLE_ID = 3000;
        inline constexpr int PLAYER_RUN_ID = 3001;
        inline constexpr int PLAYER_JUMP_ID = 3002;

        // Animation Constants
        inline constexpr int PLAYER_RUN_FRAMES = 10;
        inline constexpr int PLAYER_JUMP_FRAMES = 11;
        inline constexpr int PLAYER_FRAME_WIDTH = 64;
        inline constexpr int PLAYER_FRAME_HEIGHT = 64;
        inline constexpr float PLAYER_FRAME_DURATION = 0.1f;

    } // namespace mario::constants

