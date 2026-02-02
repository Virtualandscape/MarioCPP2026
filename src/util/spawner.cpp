#include "mario/util/Spawner.hpp"
#include "mario/world/EntitySpawn.hpp"
#include "mario/ecs/components/Position.hpp"
#include "mario/ecs/components/Velocity.hpp"
#include "mario/ecs/components/Size.hpp"
#include "mario/ecs/components/PlayerInput.hpp"
#include "mario/ecs/components/JumpState.hpp"
#include "mario/ecs/components/PlayerStats.hpp"
#include "mario/ecs/components/Type.hpp"
#include "mario/ecs/components/CollisionInfo.hpp"
#include "mario/ecs/components/EnemyComponent.hpp"
#include "mario/ecs/components/Sprite.hpp"
#include "mario/entities/PlayerConstants.hpp"
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

EntityID Spawner::spawn_player(Registry& registry, const EntitySpawn& spawn, float tile_size) {
    EntityID id = registry.create_entity();
    registry.add_component<Position>(id, {
        static_cast<float>(spawn.tile_x) * tile_size, 
        static_cast<float>(spawn.tile_y) * tile_size
    });
    registry.add_component<Velocity>(id, {0.0f, 0.0f});
    registry.add_component<Size>(id, {player::Width, player::Height});
    registry.add_component<PlayerInput>(id, {});
    registry.add_component<JumpState>(id, {});
    registry.add_component<PlayerStats>(id, {});
    registry.add_component<Type>(id, {EntityType::Player});
    registry.add_component<CollisionInfo>(id, {});
    registry.add_component<Sprite>(id, {Sprite::Shape::Ellipse, sf::Color(200, 0, 0)});
    return id;
}

EntityID Spawner::spawn_player_default(Registry& registry) {
    EntityID id = registry.create_entity();
    registry.add_component<Position>(id, {32.0f, 32.0f});
    registry.add_component<Velocity>(id, {0.0f, 0.0f});
    registry.add_component<Size>(id, {player::Width, player::Height});
    registry.add_component<PlayerInput>(id, {});
    registry.add_component<JumpState>(id, {});
    registry.add_component<PlayerStats>(id, {});
    registry.add_component<Type>(id, {EntityType::Player});
    registry.add_component<CollisionInfo>(id, {});
    registry.add_component<Sprite>(id, {Sprite::Shape::Ellipse, sf::Color(200, 0, 0)});
    return id;
}

void Spawner::spawn_enemy(Registry& registry, const EntitySpawn& spawn, float tile_size) {
    const std::string type_str = to_lower(spawn.type);
    auto entity = registry.create_entity();

    float x = static_cast<float>(spawn.tile_x) * tile_size;
    float y = static_cast<float>(spawn.tile_y) * tile_size;

    registry.add_component<Position>(entity, {x, y});
    registry.add_component<Velocity>(entity, {0.0f, 0.0f});
    registry.add_component<Size>(entity, {16.0f, 16.0f});
    registry.add_component<CollisionInfo>(entity, {});
    registry.add_component<EnemyComponent>(entity, {});

    if (type_str == "goomba") {
        registry.add_component<Type>(entity, {EntityType::Goomba});
        registry.add_component<Sprite>(entity, {Sprite::Shape::Rectangle, sf::Color::Black});
    } else if (type_str == "koopa") {
        registry.add_component<Type>(entity, {EntityType::Koopa});
        registry.add_component<Sprite>(entity, {Sprite::Shape::Rectangle, sf::Color::Red});
    }
}

} // namespace mario
