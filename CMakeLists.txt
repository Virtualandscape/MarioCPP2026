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
        src/engine/audio/audio_manager.cpp
        src/game/MarioGame.cpp
        src/game/menu_scene.cpp
        src/game/pause_scene.cpp
        src/game/play_scene.cpp
        src/engine/input/input_manager.cpp
        src/engine/render/renderer.cpp
        src/engine/resources/asset_manager.cpp
        src/game/systems/collision_system.cpp
        src/game/systems/physics_system.cpp
        src/game/systems/player_controller_system.cpp
        src/game/systems/enemy_system.cpp
        src/game/systems/level_system.cpp
        src/game/helpers/spawner.cpp
        src/game/ui/hud.cpp
        src/game/world/camera.cpp
        src/game/world/level.cpp
        src/game/world/tile_map.cpp
        src/game/helpers/TileSweep.cpp
        src/game/systems/background_system.cpp
        src/game/systems/sprite_render_system.cpp
        src/game/systems/camera_system.cpp
        src/engine/render/sprite.cpp
        src/game/gameplay/power_up.cpp
        src/game/systems/cloud_system.cpp
        src/game/systems/debug_draw_system.cpp
        src/game/systems/animation_system.cpp
        src/game/world/JsonHelper.cpp
        src/game/systems/inspector_system.cpp
        src/engine/ui/ui_manager.cpp
        src/engine/Application.cpp
        src/game/world/level.cpp
        src/engine/spatial/Quadtree.cpp
        src/engine/EngineConfig.cpp
        src/editor/EditorScene.cpp
        src/editor/EditorUI.cpp
        src/editor/EditorUI.cpp
        src/game/ui/MainMenuBar.cpp
)

set(HEADERS
        include/Zia/engine/ecs/EntityTypeComponent.hpp
        include/Zia/engine/ecs/EntityManager.hpp
        include/Zia/engine/ecs/components/PositionComponent.hpp
        include/Zia/engine/ecs/components/VelocityComponent.hpp
        include/Zia/engine/ecs/components/SizeComponent.hpp
        include/Zia/engine/ecs/components/CollisionInfoComponent.hpp
        include/Zia/engine/ecs/components/PlayerControllerComponent.hpp
        include/Zia/engine/ecs/components/TypeComponent.hpp
        include/Zia/engine/ecs/components/SpriteComponent.hpp
        include/Zia/engine/ecs/components/AnimationComponent.hpp
        include/Zia/engine/ecs/components/EnemyComponent.hpp
        include/Zia/game/systems/PlayerControllerSystem.hpp
        include/Zia/game/systems/EnemySystem.hpp
        include/Zia/game/systems/LevelSystem.hpp
        include/Zia/game/systems/AnimationSystem.hpp
        include/Zia/game/helpers/Spawner.hpp
        include/Zia/engine/ecs/components/BackgroundComponent.hpp
        include/Zia/game/systems/BackgroundSystem.hpp
        include/Zia/editor/ISceneEditor.hpp
        include/Zia/editor/EditorScene.hpp
        include/Zia/editor/EditorUI.hpp
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
