cmake_minimum_required(VERSION 3.28)
project(Mario LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

set(SOURCES
        main.cpp
        src/audio/audio_manager.cpp
        src/core/game.cpp
        src/core/menu_state.cpp
        src/core/pause_state.cpp
        src/core/play_state.cpp
        src/input/input_manager.cpp
        src/render/renderer.cpp
        src/resources/asset_manager.cpp
        src/systems/collision_system.cpp
        src/systems/physics_system.cpp
        src/systems/player_input_system.cpp
        src/systems/player_movement_system.cpp
        src/systems/enemy_system.cpp
        src/systems/level_system.cpp
        src/helpers/spawner.cpp
        src/ui/hud.cpp
        src/helpers/timer.cpp
        src/world/camera.cpp
        src/world/level.cpp
        src/world/tile_map.cpp
        src/helpers/Quadtree.cpp
        src/helpers/TileSweep.cpp
        src/systems/background_system.cpp
        src/systems/sprite_render_system.cpp
        src/systems/camera_system.cpp
        src/render/sprite.cpp
        src/gameplay/power_up.cpp
        src/systems/cloud_system.cpp
        src/systems/debug_draw_system.cpp
        src/systems/animation_system.cpp
)

set(HEADERS
        include/mario/ecs/EntityTypeComponent.hpp
        include/mario/ecs/EntityManager.hpp
        include/mario/ecs/components/PositionComponent.hpp
        include/mario/ecs/components/VelocityComponent.hpp
        include/mario/ecs/components/SizeComponent.hpp
        include/mario/ecs/components/CollisionInfoComponent.hpp
        include/mario/ecs/components/PlayerInputComponent.hpp
        include/mario/ecs/components/TypeComponent.hpp
        include/mario/ecs/components/SpriteComponent.hpp
        include/mario/ecs/components/AnimationComponent.hpp
        include/mario/ecs/components/EnemyComponent.hpp
        include/mario/systems/PlayerInputSystem.hpp
        include/mario/systems/PlayerMovementSystem.hpp
        include/mario/systems/EnemySystem.hpp
        include/mario/systems/LevelSystem.hpp
        include/mario/systems/AnimationSystem.hpp
        include/mario/helpers/Spawner.hpp
        include/mario/ecs/components/BackgroundComponent.hpp
        include/mario/systems/BackgroundSystem.hpp
)

add_executable(Mario ${SOURCES} ${HEADERS})
target_compile_features(Mario PRIVATE cxx_std_17)
target_include_directories(Mario PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Mario PRIVATE SFML::Graphics)

# Copy assets from source tree to build directory so runtime always uses up-to-date assets.
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets to build directory"
)

add_dependencies(Mario copy_assets)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
