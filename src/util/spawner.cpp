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
#include <algorithm>
#include <cctype>

namespace {
    std::string to_lower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }
}

namespace mario {
    EntityID Spawner::spawn_player(EntityManager &registry, const EntitySpawn &spawn, float tile_size) {
        using namespace mario::constants;
        EntityID id = registry.create_entity();
        registry.add_component<PositionComponent>(id, {
                                             static_cast<float>(spawn.tile_x) * tile_size,
                                             static_cast<float>(spawn.tile_y) * tile_size
                                         });
        registry.add_component<VelocityComponent>(id, {ZERO_VELOCITY, ZERO_VELOCITY});
        registry.add_component<SizeComponent>(id, SizeComponent{PLAYER_WIDTH, PLAYER_HEIGHT});
        registry.add_component<PlayerInputComponent>(id, {});
        registry.add_component<JumpStateComponent>(id, {});
        registry.add_component<PlayerStatsComponent>(id, {});
        registry.add_component<TypeComponent>(id, {EntityTypeComponent::Player});
        registry.add_component<CollisionInfoComponent>(id, {});
        registry.add_component<SpriteComponent>(id, {SpriteComponent::Shape::Ellipse, PLAYER_SPRITE_COLOR_GREEN});
        return id;
    }

    EntityID Spawner::spawn_player_default(EntityManager &registry) {
        using namespace mario::constants;
        EntityID id = registry.create_entity();
        registry.add_component<PositionComponent>(id, {PLAYER_DEFAULT_X, PLAYER_DEFAULT_Y});
        registry.add_component<VelocityComponent>(id, {ZERO_VELOCITY, ZERO_VELOCITY});
        registry.add_component<SizeComponent>(id, SizeComponent{PLAYER_WIDTH, PLAYER_HEIGHT});
        registry.add_component<PlayerInputComponent>(id, {});
        registry.add_component<JumpStateComponent>(id, {});
        registry.add_component<PlayerStatsComponent>(id, {});
        registry.add_component<TypeComponent>(id, {EntityTypeComponent::Player});
        registry.add_component<CollisionInfoComponent>(id, {});
        registry.add_component<SpriteComponent>(id, {SpriteComponent::Shape::Ellipse, PLAYER_SPRITE_COLOR_RED});
        return id;
    }

    void Spawner::spawn_enemy(EntityManager &registry, const EntitySpawn &spawn, float tile_size) {
        using namespace mario::constants;
        const std::string type_str = to_lower(spawn.type);
        auto entity = registry.create_entity();

        float x = static_cast<float>(spawn.tile_x) * tile_size;
        float y = static_cast<float>(spawn.tile_y) * tile_size;

        registry.add_component<PositionComponent>(entity, {x, y});
        registry.add_component<VelocityComponent>(entity, {ENEMY_INITIAL_SPEED, ZERO_VELOCITY}); // start moving left
        registry.add_component<SizeComponent>(entity, {ENEMY_SIZE, ENEMY_SIZE});
        registry.add_component<CollisionInfoComponent>(entity, {});
        registry.add_component<EnemyComponent>(entity, EnemyComponent{});

        if (type_str == "goomba") {
            registry.add_component<TypeComponent>(entity, {EntityTypeComponent::Goomba});
            registry.add_component<SpriteComponent>(entity, {SpriteComponent::Shape::Rectangle, ENEMY_SPRITE_COLOR_BLACK});
        } else if (type_str == "koopa") {
            registry.add_component<TypeComponent>(entity, {EntityTypeComponent::Koopa});
            registry.add_component<SpriteComponent>(entity, {SpriteComponent::Shape::Rectangle, ENEMY_SPRITE_COLOR_RED});
        }
    }
} // namespace mario
