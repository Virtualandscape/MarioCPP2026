#pragma once

#include <SFML/Graphics/Color.hpp>
#include <array>
#include <cmath>
#include <string_view>


    namespace zia::constants {
        // Project-wide fixed tile size (pixels). JSON "tileSize" is deprecated and ignored.
        inline constexpr int TILE_SIZE = 32;
        // Scale factor relative to the original tile size (16px). Use to scale movement/gravity constants so gameplay (in tiles) remains consistent.
        inline constexpr float TILE_SCALE = static_cast<float>(TILE_SIZE) / 16.0f;

        // Default gravity baseline (pixels/sec^2) scaled by TILE_SCALE. PhysicsSystem default uses this value.
        inline constexpr float DEFAULT_GRAVITY = 1200.0f * TILE_SCALE;

        // Player
        inline constexpr float PLAYER_WIDTH = 64.0f;
        inline constexpr float PLAYER_HEIGHT = 64.0f;
        inline constexpr float PLAYER_DEFAULT_X = 32.0f;
        inline constexpr float PLAYER_DEFAULT_Y = 32.0f;

        // Velocities
        inline constexpr float ZERO_VELOCITY = 0.0f;
        // Player movement defaults (scaled by tile ratio so movement in tiles/sec remains consistent)
        inline constexpr float PLAYER_MOVE_SPEED = 220.0f * TILE_SCALE;

        // Desired maximum jump height in tiles. Tweak this to change how many tiles the player can jump.
        // Previously, jump speed was 400 and gravity 1200 with 16px tiles which yields ~4.1667 tiles of jump height.
        // Use that value by default to preserve the original feel after switching to 32px tiles.
        inline constexpr float PLAYER_JUMP_TILES = 4.1666667f;

        // Compute the initial jump speed (pixels/sec) needed to reach 'tiles' tiles height under DEFAULT_GRAVITY.
        // Formula: v = sqrt(2 * g * h), where h = tiles * TILE_SIZE.
        inline float jump_speed_for_tiles(float tiles) {
            return std::sqrt(2.0f * DEFAULT_GRAVITY * (tiles * static_cast<float>(TILE_SIZE)));
        }


        // Enemy movement speed and size scaled by tile ratio
        inline constexpr float ENEMY_INITIAL_SPEED = -30.0f * TILE_SCALE;
        inline constexpr float ENEMY_SIZE = 16.0f * TILE_SCALE;

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

        // Menu background texture id base: three variants for the settings dropdown (800x600, 1024x768, 1280x720)
        inline constexpr int MENU_BACKGROUND_TEXTURE_ID = 1001;

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
        // Level paths (string_view avoids raw pointer usage for literals)
        inline constexpr std::string_view LEVEL1_PATH = "assets/levels/level1.json";
        inline constexpr std::string_view LEVEL2_PATH = "assets/levels/level2.json";

        // Central list of available levels (single definition point)
        inline constexpr std::array<std::string_view, 2> LEVEL_PATHS = { LEVEL1_PATH, LEVEL2_PATH };

        // Camera zoom scale: multiply world viewport to show more/less area. >1 = zoom out (show more tiles), <1 = zoom in.
        // Default: zoom out a bit so camera shows more area after switching to 32px tiles.
        inline constexpr float CAMERA_SCALE = 1.15f;

        // Player Textures
        inline constexpr int PLAYER_IDLE_ID = 3000;
        inline constexpr int PLAYER_RUN_ID = 3001;
        inline constexpr int PLAYER_JUMP_ID = 3002;
        // Celebrate texture id: assets/Sprites/Player64/Celebrate.png
        inline constexpr int PLAYER_CELEBRATE_ID = 3003;

        // Animation Constants
        inline constexpr int PLAYER_RUN_FRAMES = 10;
        inline constexpr int PLAYER_JUMP_FRAMES = 11;
        // Number of frames in the Celebrate sprite sheet. Adjust if the asset differs.
        inline constexpr int PLAYER_CELEBRATE_FRAMES = 11;
        // Number of Celebrate plays enqueued per stomp (default: 1)
        inline constexpr int PLAYER_CELEBRATE_REPEAT_ON_STOMP = 1;
        inline constexpr int PLAYER_FRAME_WIDTH = 64;
        inline constexpr int PLAYER_FRAME_HEIGHT = 64;
        inline constexpr float PLAYER_FRAME_DURATION = 0.1f;

        // Font IDs
        inline constexpr int FONT_MONTSERRAT_BLACK_ID = 4000;

    } // namespace Zia::constants

