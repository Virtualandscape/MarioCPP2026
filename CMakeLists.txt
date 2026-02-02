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
        src/entities/enemy.cpp
        src/entities/entity.cpp
        src/entities/player.cpp
        src/gameplay/power_up.cpp
        src/input/input_manager.cpp
        src/render/renderer.cpp
        src/render/sprite.cpp
        src/resources/asset_manager.cpp
        src/systems/collision_system.cpp
        src/systems/physics_system.cpp
        src/systems/player_input_system.cpp
        src/systems/player_movement_system.cpp
        src/ui/hud.cpp
        src/util/timer.cpp
        src/world/camera.cpp
        src/world/level.cpp
        src/world/tile_map.cpp
        )

set(HEADERS
        include/mario/ecs/EntityType.hpp
        include/mario/ecs/Registry.hpp
        include/mario/ecs/components/Position.hpp
        include/mario/ecs/components/Velocity.hpp
        include/mario/ecs/components/Size.hpp
        include/mario/ecs/components/CollisionInfo.hpp
        include/mario/ecs/components/PlayerInput.hpp
        include/mario/ecs/components/JumpState.hpp
        include/mario/ecs/components/PlayerStats.hpp
        include/mario/ecs/components/Type.hpp
        include/mario/systems/PlayerInputSystem.hpp
        include/mario/systems/PlayerMovementSystem.hpp
        src/systems/player_movement_system.cpp
)

add_executable(Mario ${SOURCES} ${HEADERS})
target_compile_features(Mario PRIVATE cxx_std_17)
target_include_directories(Mario PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Mario PRIVATE SFML::Graphics)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
