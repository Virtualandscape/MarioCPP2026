#include "mario/systems/CloudSystem.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/ecs/components/CloudComponent.hpp"
#include "mario/util/Spawner.hpp"
#include "mario/util/Constants.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <algorithm> // For std::sort

namespace mario {

void CloudSystem::initialize(AssetManager& assets, EntityManager& registry) {
    Spawner::spawn_clouds(registry, assets);
}

void CloudSystem::update(EntityManager& registry, float dt) {
    using namespace mario::constants;
    static thread_local std::vector<EntityID> entities;
    registry.get_entities_with<CloudComponent>(entities);
    for (auto entity : entities) {
        auto* cloud = registry.get_component<CloudComponent>(entity);
        if (!cloud) continue;
        // Move cloud to the right
        cloud->x += cloud->speed * dt;
        // If off screen, reset to left
        if (cloud->x > CLOUD_RESET_X) {
            cloud->x = CLOUD_SPAWN_X;
        }
    }
}

void CloudSystem::render(Renderer& renderer, const Camera& camera, AssetManager& assets, EntityManager& registry) {
    using namespace mario::constants;
    static thread_local std::vector<EntityID> entities;
    registry.get_entities_with<CloudComponent>(entities);
    // Sort by layer: Big first, then Medium, then Small (so Small on top)
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

        sf::Sprite sprite(*tex);
        sprite.setScale({cloud->scale, cloud->scale});
        // Position relative to camera for parallax
        float pos_x = cloud->x - camera.x() * CLOUD_PARALLAX;
        float pos_y = cloud->y;
        sprite.setPosition({pos_x, pos_y});
        window.draw(sprite);
    }

    window.setView(old_view);
}

} // namespace mario
