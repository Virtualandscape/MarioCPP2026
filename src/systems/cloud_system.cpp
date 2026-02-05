// Implements the CloudSystem, which manages cloud entity positioning, rendering, and parallax effects.
// Follows ECS pattern: queries cloud components and updates/renders them independently.

#include "mario/systems/CloudSystem.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/ecs/components/CloudComponent.hpp"
#include "mario/helpers/Spawner.hpp"
#include "mario/helpers/Constants.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <algorithm>

namespace mario {

// Initializes cloud entities by spawning them via Spawner utility.
void CloudSystem::initialize(AssetManager& assets, EntityManager& registry) {
    Spawner::spawn_clouds(registry, assets);
}

// Updates cloud positions: moves them horizontally and resets off-screen clouds.
void CloudSystem::update(EntityManager& registry, float dt) {
    using namespace mario::constants;
    static thread_local std::vector<EntityID> entities;
    registry.get_entities_with<CloudComponent>(entities);

    for (auto entity : entities) {
        auto* cloud = registry.get_component<CloudComponent>(entity);
        if (!cloud) continue;

        // Move cloud horizontally based on speed and delta time
        cloud->x += cloud->speed * dt;

        // Reset cloud to spawn position when it goes off-screen
        if (cloud->x > CLOUD_RESET_X) {
            cloud->x = CLOUD_SPAWN_X;
        }
    }
}

// Renders all cloud entities with proper layering and parallax offset.
void CloudSystem::render(Renderer& renderer, const Camera& camera, AssetManager& assets, EntityManager& registry) {
    using namespace mario::constants;
    static thread_local std::vector<EntityID> entities;
    registry.get_entities_with<CloudComponent>(entities);

    // Sort clouds by layer for correct depth ordering (Big < Medium < Small means Big renders first)
    std::sort(entities.begin(), entities.end(), [&](EntityID a, EntityID b) {
        auto* ca = registry.get_component<CloudComponent>(a);
        auto* cb = registry.get_component<CloudComponent>(b);
        if (!ca || !cb) return false;
        return static_cast<int>(ca->layer) < static_cast<int>(cb->layer);
    });

    sf::RenderWindow& window = renderer.window();
    const sf::View old_view = window.getView();
    window.setView(window.getDefaultView());

    for (auto entity : entities) {
        auto* cloud = registry.get_component<CloudComponent>(entity);
        if (!cloud) continue;

        sf::Texture* tex = assets.get_mutable_texture(cloud->texture_id);
        if (!tex) continue;

        // Create sprite and apply scale
        sf::Sprite sprite(*tex);
        sprite.setScale({cloud->scale, cloud->scale});

        // Position with parallax effect: clouds move less than camera
        float pos_x = cloud->x - camera.x() * CLOUD_PARALLAX;
        float pos_y = cloud->y;
        sprite.setPosition({pos_x, pos_y});
        window.draw(sprite);
    }

    window.setView(old_view);
}

} // namespace mario
