cmake_minimum_required(VERSION 3.28)
project(Mario LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(SFML_BUILD_AUDIO OFF CACHE BOOL "Disable SFML audio to avoid extra dependencies" FORCE)
set(SFML_BUILD_CARDS OFF CACHE BOOL "Optional SFML module" FORCE)

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.1
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_MakeAvailable(imgui)
endif()

FetchContent_Declare(imgui-sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v3.0
)

set(IMGUI_DIR ${imgui_SOURCE_DIR} CACHE PATH "imgui top-level directory" FORCE)
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(imgui-sfml)

set(SOURCES
        main.cpp
        src/audio/audio_manager.cpp
        src/core/game.cpp
        src/game/menu_scene.cpp
        src/game/pause_scene.cpp
        src/game/play_scene.cpp
        src/input/input_manager.cpp
        src/render/renderer.cpp
        src/resources/asset_manager.cpp
        src/systems/collision_system.cpp
        src/systems/physics_system.cpp
        src/systems/player_controller_system.cpp
        src/systems/enemy_system.cpp
        src/systems/level_system.cpp
        src/helpers/spawner.cpp
        src/ui/hud.cpp
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
        src/world/JsonHelper.cpp
        src/systems/inspector_system.cpp
        src/engine/ui_manager.cpp
        src/engine/Application.cpp
        src/engine/ui_manager.cpp
)

set(HEADERS
        include/mario/ecs/EntityTypeComponent.hpp
        include/mario/ecs/EntityManager.hpp
        include/mario/ecs/components/PositionComponent.hpp
        include/mario/ecs/components/VelocityComponent.hpp
        include/mario/ecs/components/SizeComponent.hpp
        include/mario/ecs/components/CollisionInfoComponent.hpp
        include/mario/ecs/components/PlayerControllerComponent.hpp
        include/mario/ecs/components/TypeComponent.hpp
        include/mario/ecs/components/SpriteComponent.hpp
        include/mario/ecs/components/AnimationComponent.hpp
        include/mario/ecs/components/EnemyComponent.hpp
        include/mario/systems/PlayerControllerSystem.hpp
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
target_link_libraries(Mario PRIVATE SFML::Graphics ImGui-SFML::ImGui-SFML)

# Copy assets from source tree to build directory so runtime always uses up-to-date assets.
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets to build directory"
)

add_dependencies(Mario copy_assets)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
