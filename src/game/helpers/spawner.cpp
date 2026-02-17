// This file implements the Spawner utility for creating (spawning) player and enemy entities in the game.
// Spawner is a factory pattern implementation for ECS: it constructs entities with proper component composition.
// Following ECS best practices: each spawned entity has all required components for the systems that will operate on it.

#include "Zia/game/helpers/Spawner.hpp"
#include "Zia/game/helpers/Constants.hpp"
#include "Zia/game/world/EntitySpawn.hpp"
#include "Zia/engine/ecs/components/PositionComponent.hpp"
#include "Zia/engine/ecs/components/VelocityComponent.hpp"
#include "Zia/engine/ecs/components/SizeComponent.hpp"
#include "Zia/engine/ecs/components/PlayerControllerComponent.hpp"
#include "Zia/engine/ecs/components/TypeComponent.hpp"
#include "Zia/engine/ecs/components/CollisionInfoComponent.hpp"
#include "Zia/engine/ecs/components/EnemyComponent.hpp"
#include "Zia/engine/ecs/components/SpriteComponent.hpp"
#include "Zia/engine/ecs/components/AnimationComponent.hpp"
#include "Zia/engine/ecs/components/CloudComponent.hpp"
#include "Zia/engine/resources/AssetManager.hpp"
#include <algorithm>
#include <cctype>
#include <random>

// Anonymous namespace for internal utility functions (avoid polluting global namespace)
namespace {
    // Used by: Spawner::spawn_enemy (internal helper)
    // Converts a string to lowercase for case-insensitive comparison
    std::string to_lower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }
}

namespace zia {
    // Used by: PlayState (level spawning), systems that need to spawn a player dynamically
    // Spawns a player entity at a tile position with all required components.
    // Component composition: Position, Velocity, Size, Input, JumpState, PlayerStats, Type, Collision, Sprite.
    // This ensures the entity will be correctly processed by all relevant systems (movement, physics, input, collision, render).
    EntityID Spawner::spawn_player(zia::engine::IEntityManager &registry, const EntitySpawn &spawn, zia::engine::IAssetManager& assets) {
        using namespace zia::constants;

        // Do not load textures here during spawn; assume assets are preloaded by the caller.
        // assets.load_texture(PLAYER_IDLE_ID, "assets/Sprites/Player64/Idle.png");
        // assets.load_texture(PLAYER_RUN_ID, "assets/Sprites/Player64/Run.png");
        // assets.load_texture(PLAYER_JUMP_ID, "assets/Sprites/Player64/Jump.png");

        EntityID id = registry.create_entity();

        // Spatial component: position from tile coordinates
        // Adjust spawn Y so the player's feet are aligned with the tile row.
        const auto tile_size = static_cast<float>(TILE_SIZE);
        const float spawn_px = static_cast<float>(spawn.tile_x) * tile_size;
        const float spawn_py = static_cast<float>(spawn.tile_y) * tile_size - (PLAYER_HEIGHT - tile_size);
        registry.add_component<PositionComponent>(id, { spawn_px, spawn_py });

        // Physics components: velocity and size (physical hitbox narrower than rendered sprite)
        registry.add_component<VelocityComponent>(id, {ZERO_VELOCITY, ZERO_VELOCITY});
        // Use a narrower physical width (half of visual width) to tighten collision box
        registry.add_component<SizeComponent>(id, {PLAYER_WIDTH * 0.5f, PLAYER_HEIGHT});

        // Gameplay components: input, jump state, player movement parameters
        // PlayerInputComponent contains input and jump_count now. Movement uses global constants.
        registry.add_component<PlayerControllerComponent>(id, {});

        // ECS metadata: entity type and collision info
        registry.add_component<TypeComponent>(id, {EntityTypeComponent::Player});
        registry.add_component<CollisionInfoComponent>(id, {});

        // Rendering component: visual representation
        SpriteComponent sc;
        sc.texture_id = PLAYER_IDLE_ID;
        // Draw the sprite at full 64x64 while hitbox is narrower
        sc.render_size = { PLAYER_WIDTH, PLAYER_HEIGHT };
        // Center rendered sprite horizontally over the physical hitbox
        sc.render_offset = { -(PLAYER_WIDTH - (PLAYER_WIDTH * 0.5f)) * 0.5f, 0.0f };
        // Set initial texture_rect for Idle state
        sc.texture_rect = sf::IntRect({0, 0}, {PLAYER_FRAME_WIDTH, PLAYER_FRAME_HEIGHT});
        registry.add_component<SpriteComponent>(id, sc);

        // Animation component
        registry.add_component<AnimationComponent>(id, {});

        return id;
    }

    // Used by: PlayState (fallback when explicit spawn not found)
    // Spawns a player entity at the default position (used for initial/fallback spawning).
    // Same component composition as spawn_player but at fixed coordinates.
    EntityID Spawner::spawn_player_default(zia::engine::IEntityManager &registry, zia::engine::IAssetManager& assets) {
        using namespace zia::constants;

        // Do not load textures here during spawn; assume the caller preloads assets.
        // assets.load_texture(PLAYER_IDLE_ID, "assets/Sprites/Player64/Idle.png");
        // assets.load_texture(PLAYER_RUN_ID, "assets/Sprites/Player64/Run.png");
        // assets.load_texture(PLAYER_JUMP_ID, "assets/Sprites/Player64/Jump.png");

        EntityID id = registry.create_entity();

        // Spatial component: default position
        // For default spawn, keep the previous default X / Y but adjust so feet align when using project tile size.
        const auto tile_size = static_cast<float>(TILE_SIZE);
        registry.add_component<PositionComponent>(id, {PLAYER_DEFAULT_X, PLAYER_DEFAULT_Y - (PLAYER_HEIGHT - tile_size)});

        // Physics components: velocity and size for default spawn
        registry.add_component<VelocityComponent>(id, {ZERO_VELOCITY, ZERO_VELOCITY});
        registry.add_component<SizeComponent>(id, {PLAYER_WIDTH * 0.5f, PLAYER_HEIGHT});

        // Gameplay components: input, jump state, player movement parameters
        registry.add_component<PlayerControllerComponent>(id, {});

        // ECS metadata: entity type and collision info
        registry.add_component<TypeComponent>(id, {EntityTypeComponent::Player});
        registry.add_component<CollisionInfoComponent>(id, {});

        // Rendering component: visual representation for default spawn
        SpriteComponent sc;
        sc.texture_id = PLAYER_IDLE_ID;
        sc.render_size = { PLAYER_WIDTH, PLAYER_HEIGHT };
        sc.render_offset = { -(PLAYER_WIDTH - (PLAYER_WIDTH * 0.5f)) * 0.5f, 0.0f };
        // Set initial texture_rect for Idle state
        sc.texture_rect = sf::IntRect({0, 0}, {PLAYER_FRAME_WIDTH, PLAYER_FRAME_HEIGHT});
        registry.add_component<SpriteComponent>(id, sc);

        // Animation component
        registry.add_component<AnimationComponent>(id, {});

         return id;
     }

    // Used by: PlayState (level spawning)
    // Spawns an enemy entity at a tile position with type-specific rendering.
    // Component composition: Position, Velocity, Size, Collision, Enemy, Type, Sprite.
    // Enemies follow platforms and reverse direction on collision (see EnemySystem).
    void Spawner::spawn_enemy(zia::engine::IEntityManager &registry, const EntitySpawn &spawn) {
        using namespace zia::constants;

        const std::string type_str = to_lower(spawn.type);
        EntityID entity = registry.create_entity();

        const auto tile_size = static_cast<float>(TILE_SIZE);
        float x = static_cast<float>(spawn.tile_x) * tile_size;
        float y = static_cast<float>(spawn.tile_y) * tile_size;

        // Spatial components: position and size
        registry.add_component<PositionComponent>(entity, {x, y});
        registry.add_component<SizeComponent>(entity, {ENEMY_SIZE, ENEMY_SIZE});

        // Physics component: initial velocity (moving left)
        registry.add_component<VelocityComponent>(entity, {ENEMY_INITIAL_SPEED, ZERO_VELOCITY});

        // ECS metadata: collision info and enemy marker
        registry.add_component<CollisionInfoComponent>(entity, {});
        registry.add_component<EnemyComponent>(entity, {});

        // Type and sprite are set based on enemy type string
        if (type_str == "goomba") {
            registry.add_component<TypeComponent>(entity, {EntityTypeComponent::Goomba});
            registry.add_component<SpriteComponent>(entity, {SpriteComponent::Shape::Rectangle, ENEMY_SPRITE_COLOR_BLACK});
        } else if (type_str == "koopa") {
            registry.add_component<TypeComponent>(entity, {EntityTypeComponent::Koopa});
            registry.add_component<SpriteComponent>(entity, {SpriteComponent::Shape::Rectangle, ENEMY_SPRITE_COLOR_RED});
        }
    }

    // Used by: CloudSystem (initialization)
    // Spawns all cloud entities with randomized positions and loads textures.
    // Creates three layers of clouds (Big, Medium, Small) for parallax depth effect.
    void Spawner::spawn_clouds(zia::engine::IEntityManager& registry, zia::engine::IAssetManager& assets) {
        using namespace zia::constants;

        // Load cloud textures into asset manager
        // assets.load_texture(CLOUD_BIG_ID, "assets/environment/background/cloud_big.png");
        // assets.load_texture(CLOUD_MEDIUM_ID, "assets/environment/background/cloud_medium.png");
        // assets.load_texture(CLOUD_SMALL_ID, "assets/environment/background/cloud_small.png");

        // Set up random distribution for cloud Y positions
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> big_y_dist(CLOUD_BIG_Y_MIN, CLOUD_BIG_Y_MAX);
        std::uniform_real_distribution<float> med_y_dist(CLOUD_MEDIUM_Y_MIN, CLOUD_MEDIUM_Y_MAX);
        std::uniform_real_distribution<float> small_y_dist(CLOUD_SMALL_Y_MIN, CLOUD_SMALL_Y_MAX);

        // Create a Big clouds layer
        for (int i = 0; i < NUM_BIG_CLOUDS; ++i) {
            auto id = registry.create_entity();
            CloudComponent cc;
            cc.texture_id = CLOUD_BIG_ID;
            cc.layer = CloudComponent::Layer::Big;
            cc.speed = CLOUD_BIG_SPEED;
            cc.x = CLOUD_SPAWN_X - static_cast<float>(i) * CLOUD_BIG_SPACING;
            cc.y = big_y_dist(gen);
            cc.scale = CLOUD_SCALE;
            registry.add_component(id, cc);
        }

        // Create Medium clouds layer
        for (int i = 0; i < NUM_MEDIUM_CLOUDS; ++i) {
            auto id = registry.create_entity();
            CloudComponent cc;
            cc.texture_id = CLOUD_MEDIUM_ID;
            cc.layer = CloudComponent::Layer::Medium;
            cc.speed = CLOUD_MEDIUM_SPEED;
            cc.x = CLOUD_SPAWN_X - static_cast<float>(i) * CLOUD_MEDIUM_SPACING;
            cc.y = med_y_dist(gen);
            cc.scale = CLOUD_SCALE;
            registry.add_component(id, cc);
        }

        // Create a Small clouds layer
        for (int i = 0; i < NUM_SMALL_CLOUDS; ++i) {
            auto id = registry.create_entity();
            CloudComponent cc;
            cc.texture_id = CLOUD_SMALL_ID;
            cc.layer = CloudComponent::Layer::Small;
            cc.speed = CLOUD_SMALL_SPEED;

            cc.x = CLOUD_SPAWN_X - static_cast<float>(i) * CLOUD_SMALL_SPACING;
            cc.y = small_y_dist(gen);
            cc.scale = CLOUD_SCALE;
            registry.add_component(id, cc);
        }
    }
} // namespace Zia
