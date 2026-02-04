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
        src/util/spawner.cpp
        src/ui/hud.cpp
        src/util/timer.cpp
        src/world/camera.cpp
        src/world/level.cpp
        src/world/tile_map.cpp
        src/systems/player_movement_system.cpp
        src/util/Quadtree.cpp
        src/util/TileSweep.cpp
        src/systems/background_system.cpp
)

set(HEADERS
        include/mario/ecs/EntityTypeComponent.hpp
        include/mario/ecs/EntityManager.hpp
        include/mario/ecs/components/PositionComponent.hpp
        include/mario/ecs/components/VelocityComponent.hpp
        include/mario/ecs/components/SizeComponent.hpp
        include/mario/ecs/components/CollisionInfoComponent.hpp
        include/mario/ecs/components/PlayerInputComponent.hpp
        include/mario/ecs/components/JumpStateComponent.hpp
        include/mario/ecs/components/PlayerStatsComponent.hpp
        include/mario/ecs/components/TypeComponent.hpp
        include/mario/ecs/components/SpriteComponent.hpp
        include/mario/ecs/components/EnemyComponent.hpp
        include/mario/systems/PlayerInputSystem.hpp
        include/mario/systems/PlayerMovementSystem.hpp
        include/mario/systems/EnemySystem.hpp
        include/mario/systems/LevelSystem.hpp
        include/mario/util/Spawner.hpp
        include/mario/ecs/components/BackgroundComponent.hpp
        include/mario/systems/BackgroundSystem.hpp
        src/util/TileSweep.cpp
)

add_executable(Mario ${SOURCES} ${HEADERS})
target_compile_features(Mario PRIVATE cxx_std_17)
target_include_directories(Mario PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Mario PRIVATE SFML::Graphics)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
