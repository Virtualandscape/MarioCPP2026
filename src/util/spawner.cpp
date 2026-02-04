// This file implements the Spawner utility for creating (spawning) player and enemy entities in the game.
// It provides functions to initialize entities with the correct components and properties for gameplay.

#include "mario/util/Spawner.hpp"
#include "mario/util/Constants.hpp"
#include "mario/world/EntitySpawn.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/ecs/components/PlayerInputComponent.hpp"
#include "mario/ecs/components/JumpStateComponent.hpp"
#include "mario/ecs/components/PlayerStatsComponent.hpp"
#include "mario/ecs/components/TypeComponent.hpp"
#include "mario/ecs/components/CollisionInfoComponent.hpp"
#include "mario/ecs/components/EnemyComponent.hpp"
#include "mario/ecs/components/SpriteComponent.hpp"
#include "mario/ecs/components/CloudComponent.hpp"
#include "mario/resources/AssetManager.hpp"
#include <algorithm>
#include <cctype>
#include <random>

// Anonymous namespace for internal utility functions
namespace {
    // Converts a string to lowercase (used for enemy type comparison)
    std::string to_lower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }
}

namespace mario {
    // Spawns a player entity at a given tile position, initializing all required components
    EntityID Spawner::spawn_player(EntityManager &registry, const EntitySpawn &spawn, float tile_size) {
        using namespace mario::constants;
        EntityID id = registry.create_entity();
        // Set player position based on tile coordinates
        registry.add_component<PositionComponent>(id, {
                                             static_cast<float>(spawn.tile_x) * tile_size,
                                             static_cast<float>(spawn.tile_y) * tile_size
                                         });
        // Initialize velocity, size, input, jump state, stats, type, collision, and sprite
        registry.add_component<VelocityComponent>(id, {ZERO_VELOCITY, ZERO_VELOCITY});
        registry.add_component<SizeComponent>(id, SizeComponent{PLAYER_WIDTH, PLAYER_HEIGHT});
        registry.add_component<PlayerInputComponent>(id, {});
        registry.add_component<JumpStateComponent>(id, {});
        registry.add_component<PlayerStatsComponent>(id, {});
        registry.add_component<TypeComponent>(id, {EntityTypeComponent::Player});
        registry.add_component<CollisionInfoComponent>(id, {});
        // Use a green ellipse to represent the player sprite
        registry.add_component<SpriteComponent>(id, {SpriteComponent::Shape::Ellipse, PLAYER_SPRITE_COLOR_GREEN});
        return id;
    }

    // Spawns a player entity at the default position, used for initial game setup
    EntityID Spawner::spawn_player_default(EntityManager &registry) {
        using namespace mario::constants;
        EntityID id = registry.create_entity();
        // Set player position to default coordinates
        registry.add_component<PositionComponent>(id, {PLAYER_DEFAULT_X, PLAYER_DEFAULT_Y});
        // Initialize velocity, size, input, jump state, stats, type, collision, and sprite
        registry.add_component<VelocityComponent>(id, {ZERO_VELOCITY, ZERO_VELOCITY});
        registry.add_component<SizeComponent>(id, SizeComponent{PLAYER_WIDTH, PLAYER_HEIGHT});
        registry.add_component<PlayerInputComponent>(id, {});
        registry.add_component<JumpStateComponent>(id, {});
        registry.add_component<PlayerStatsComponent>(id, {});
        registry.add_component<TypeComponent>(id, {EntityTypeComponent::Player});
        registry.add_component<CollisionInfoComponent>(id, {});
        // Use a red ellipse to represent the player sprite
        registry.add_component<SpriteComponent>(id, {SpriteComponent::Shape::Ellipse, PLAYER_SPRITE_COLOR_RED});
        return id;
    }

    // Spawns an enemy entity at a given tile position, with type determined by the spawn data
    void Spawner::spawn_enemy(EntityManager &registry, const EntitySpawn &spawn, float tile_size) {
        using namespace mario::constants;
        // Convert enemy type string to lowercase for comparison
        const std::string type_str = to_lower(spawn.type);
        auto entity = registry.create_entity();

        float x = static_cast<float>(spawn.tile_x) * tile_size;
        float y = static_cast<float>(spawn.tile_y) * tile_size;

        // Set enemy position, velocity (moving left), size, collision, and enemy marker
        registry.add_component<PositionComponent>(entity, {x, y});
        registry.add_component<VelocityComponent>(entity, {ENEMY_INITIAL_SPEED, ZERO_VELOCITY}); // start moving left
        registry.add_component<SizeComponent>(entity, {ENEMY_SIZE, ENEMY_SIZE});
        registry.add_component<CollisionInfoComponent>(entity, {});
        registry.add_component<EnemyComponent>(entity, EnemyComponent{});

        // Set enemy type and sprite color/shape based on type
        if (type_str == "goomba") {
            registry.add_component<TypeComponent>(entity, {EntityTypeComponent::Goomba});
            registry.add_component<SpriteComponent>(entity, {SpriteComponent::Shape::Rectangle, ENEMY_SPRITE_COLOR_BLACK});
        } else if (type_str == "koopa") {
            registry.add_component<TypeComponent>(entity, {EntityTypeComponent::Koopa});
            registry.add_component<SpriteComponent>(entity, {SpriteComponent::Shape::Rectangle, ENEMY_SPRITE_COLOR_RED});
        }
    }

    // Spawns cloud entities with random Y positions and loads their textures
    void Spawner::spawn_clouds(EntityManager& registry, AssetManager& assets) {
        using namespace mario::constants;

        // Load cloud textures
        assets.load_texture(CLOUD_BIG_ID, "assets/environment/background/cloud_big.png");
        assets.load_texture(CLOUD_MEDIUM_ID, "assets/environment/background/cloud_medium.png");
        assets.load_texture(CLOUD_SMALL_ID, "assets/environment/background/cloud_small.png");

        // Random Y positions
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> big_y_dist(CLOUD_BIG_Y_MIN, CLOUD_BIG_Y_MAX);
        std::uniform_real_distribution<float> med_y_dist(CLOUD_MEDIUM_Y_MIN, CLOUD_MEDIUM_Y_MAX);
        std::uniform_real_distribution<float> small_y_dist(CLOUD_SMALL_Y_MIN, CLOUD_SMALL_Y_MAX);

        // Create cloud entities
        // Big clouds
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
        // Medium clouds
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
        // Small clouds
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
