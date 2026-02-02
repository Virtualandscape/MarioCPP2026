#include "mario/util/Spawner.hpp"
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
    EntityID Spawner::spawn_player(Registry &registry, const EntitySpawn &spawn, float tile_size) {
        EntityID id = registry.create_entity();
        registry.add_component<PositionComponent>(id, {
                                             static_cast<float>(spawn.tile_x) * tile_size,
                                             static_cast<float>(spawn.tile_y) * tile_size
                                         });
        registry.add_component<VelocityComponent>(id, {0.0f, 0.0f});
        registry.add_component<SizeComponent>(id, {player::Width, player::Height});
        registry.add_component<PlayerInputComponent>(id, {});
        registry.add_component<JumpStateComponent>(id, {});
        registry.add_component<PlayerStatsComponent>(id, {});
        registry.add_component<TypeComponent>(id, {EntityTypeComponent::Player});
        registry.add_component<CollisionInfoComponent>(id, {});
        registry.add_component<SpriteComponent>(id, {SpriteComponent::Shape::Ellipse, sf::Color(200, 0, 0)});
        return id;
    }

    EntityID Spawner::spawn_player_default(Registry &registry) {
        EntityID id = registry.create_entity();
        registry.add_component<PositionComponent>(id, {32.0f, 32.0f});
        registry.add_component<VelocityComponent>(id, {0.0f, 0.0f});
        registry.add_component<SizeComponent>(id, {player::Width, player::Height});
        registry.add_component<PlayerInputComponent>(id, {});
        registry.add_component<JumpStateComponent>(id, {});
        registry.add_component<PlayerStatsComponent>(id, {});
        registry.add_component<TypeComponent>(id, {EntityTypeComponent::Player});
        registry.add_component<CollisionInfoComponent>(id, {});
        registry.add_component<SpriteComponent>(id, {SpriteComponent::Shape::Ellipse, sf::Color(200, 0, 0)});
        return id;
    }

    void Spawner::spawn_enemy(Registry &registry, const EntitySpawn &spawn, float tile_size) {
        const std::string type_str = to_lower(spawn.type);
        auto entity = registry.create_entity();

        float x = static_cast<float>(spawn.tile_x) * tile_size;
        float y = static_cast<float>(spawn.tile_y) * tile_size;

        registry.add_component<PositionComponent>(entity, {x, y});
        registry.add_component<VelocityComponent>(entity, {0.0f, 0.0f});
        registry.add_component<SizeComponent>(entity, {16.0f, 16.0f});
        registry.add_component<CollisionInfoComponent>(entity, {});
        registry.add_component<EnemyComponent>(entity, {});

        if (type_str == "goomba") {
            registry.add_component<TypeComponent>(entity, {EntityTypeComponent::Goomba});
            registry.add_component<SpriteComponent>(entity, {SpriteComponent::Shape::Rectangle, sf::Color::Black});
        } else if (type_str == "koopa") {
            registry.add_component<TypeComponent>(entity, {EntityTypeComponent::Koopa});
            registry.add_component<SpriteComponent>(entity, {SpriteComponent::Shape::Rectangle, sf::Color::Red});
        }
    }
} // namespace mario
