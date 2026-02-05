// This file implements the Spawner utility for creating (spawning) player and enemy entities in the game.
// Spawner is a factory pattern implementation for ECS: it constructs entities with proper component composition.
// Following ECS best practices: each spawned entity has all required components for the systems that will operate on it.

#include "mario/helpers/Spawner.hpp"
#include "mario/helpers/Constants.hpp"
#include "mario/world/EntitySpawn.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/ecs/components/PlayerInputComponent.hpp"
#include "mario/ecs/components/TypeComponent.hpp"
#include "mario/ecs/components/CollisionInfoComponent.hpp"
#include "mario/ecs/components/EnemyComponent.hpp"
#include "mario/ecs/components/SpriteComponent.hpp"
#include "mario/ecs/components/CloudComponent.hpp"
#include "mario/resources/AssetManager.hpp"
#include <algorithm>
#include <cctype>
#include <random>

// Anonymous namespace for internal utility functions (avoid polluting global namespace)
namespace {
    // Converts a string to lowercase for case-insensitive comparison
    std::string to_lower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }
}

namespace mario {
    // Spawns a player entity at a tile position with all required components.
    // Component composition: Position, Velocity, Size, Input, JumpState, PlayerStats, Type, Collision, Sprite.
    // This ensures the entity will be correctly processed by all relevant systems (movement, physics, input, collision, render).
    EntityID Spawner::spawn_player(EntityManager &registry, const EntitySpawn &spawn, float tile_size, AssetManager& assets) {
        using namespace mario::constants;

        assets.load_texture(PLAYER_IDLE_ID, "assets/Sprites/Player/Idle.png");

        EntityID id = registry.create_entity();

        // Spatial component: position from tile coordinates
        registry.add_component<PositionComponent>(id, {
            static_cast<float>(spawn.tile_x) * tile_size,
            static_cast<float>(spawn.tile_y) * tile_size
        });

        // Physics components: velocity and size
        registry.add_component<VelocityComponent>(id, {ZERO_VELOCITY, ZERO_VELOCITY});
        registry.add_component<SizeComponent>(id, {PLAYER_WIDTH, PLAYER_HEIGHT});

        // Gameplay components: input, jump state, player movement parameters
        // PlayerInputComponent contains input and jump_count now. Movement uses global constants.
        registry.add_component<PlayerInputComponent>(id, {});

        // ECS metadata: entity type and collision info
        registry.add_component<TypeComponent>(id, {EntityTypeComponent::Player});
        registry.add_component<CollisionInfoComponent>(id, {});

        // Rendering component: visual representation
        SpriteComponent sc;
        sc.texture_id = PLAYER_IDLE_ID;
        sc.render_offset = {-8.0f, -8.0f};
        registry.add_component<SpriteComponent>(id, sc);

        return id;
    }

    // Spawns a player entity at the default position (used for initial/fallback spawning).
    // Same component composition as spawn_player but at fixed coordinates.
    EntityID Spawner::spawn_player_default(EntityManager &registry, AssetManager& assets) {
        using namespace mario::constants;

        assets.load_texture(PLAYER_IDLE_ID, "assets/Sprites/Player/Idle.png");

        EntityID id = registry.create_entity();

        // Spatial component: default position
        registry.add_component<PositionComponent>(id, {PLAYER_DEFAULT_X, PLAYER_DEFAULT_Y});

        // Physics components: velocity and size
        registry.add_component<VelocityComponent>(id, {ZERO_VELOCITY, ZERO_VELOCITY});
        registry.add_component<SizeComponent>(id, {PLAYER_WIDTH, PLAYER_HEIGHT});

        // Gameplay components: input, jump state, player movement parameters
        registry.add_component<PlayerInputComponent>(id, {});

        // ECS metadata: entity type and collision info
        registry.add_component<TypeComponent>(id, {EntityTypeComponent::Player});
        registry.add_component<CollisionInfoComponent>(id, {});

        // Rendering component: visual representation
        SpriteComponent sc;
        sc.texture_id = PLAYER_IDLE_ID;
        sc.render_offset = {-8.0f, -8.0f};
        registry.add_component<SpriteComponent>(id, sc);

        return id;
    }

    // Spawns an enemy entity at a tile position with type-specific rendering.
    // Component composition: Position, Velocity, Size, Collision, Enemy, Type, Sprite.
    // Enemies follow platforms and reverse direction on collision (see EnemySystem).
    void Spawner::spawn_enemy(EntityManager &registry, const EntitySpawn &spawn, float tile_size) {
        using namespace mario::constants;

        const std::string type_str = to_lower(spawn.type);
        EntityID entity = registry.create_entity();

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

    // Spawns all cloud entities with randomized positions and loads textures.
    // Creates three layers of clouds (Big, Medium, Small) for parallax depth effect.
    void Spawner::spawn_clouds(EntityManager& registry, AssetManager& assets) {
        using namespace mario::constants;

        // Load cloud textures into asset manager
        assets.load_texture(CLOUD_BIG_ID, "assets/environment/background/cloud_big.png");
        assets.load_texture(CLOUD_MEDIUM_ID, "assets/environment/background/cloud_medium.png");
        assets.load_texture(CLOUD_SMALL_ID, "assets/environment/background/cloud_small.png");

        // Set up random distribution for cloud Y positions
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> big_y_dist(CLOUD_BIG_Y_MIN, CLOUD_BIG_Y_MAX);
        std::uniform_real_distribution<float> med_y_dist(CLOUD_MEDIUM_Y_MIN, CLOUD_MEDIUM_Y_MAX);
        std::uniform_real_distribution<float> small_y_dist(CLOUD_SMALL_Y_MIN, CLOUD_SMALL_Y_MAX);

        // Create Big clouds layer
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

        // Create Small clouds layer
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
} // namespace mario
