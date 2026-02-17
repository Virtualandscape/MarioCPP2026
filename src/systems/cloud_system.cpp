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
    void CloudSystem::initialize(mario::engine::IAssetManager &assets, mario::engine::IEntityManager &registry) {
        // Spawner now expects IEntityManager + IAssetManager; forward registry and assets
        Spawner::spawn_clouds(registry, assets);
    }

    // Updates cloud positions: moves them horizontally and resets off-screen clouds.
    void CloudSystem::update(mario::engine::IEntityManager &registry, float dt) {
        using namespace mario::constants;
        static thread_local std::vector<EntityID> entities;
        registry.get_entities_with<CloudComponent>(entities);

        for (auto entity: entities) {
            auto cloud_opt = registry.get_component<CloudComponent>(entity);
            if (!cloud_opt) continue;
            auto &cloud = cloud_opt->get();

            // Move cloud horizontally based on speed and delta time
            cloud.x += cloud.speed * dt;

            // Reset cloud to spawn position when it goes off-screen
            if (cloud.x > CLOUD_RESET_X) {
                cloud.x = CLOUD_SPAWN_X;
            }
        }
    }

    // Renders all cloud entities with proper layering and parallax offset.
    void CloudSystem::render(mario::engine::IRenderer &renderer, const Camera &camera, mario::engine::IAssetManager &assets, mario::engine::IEntityManager &registry) {
         using namespace mario::constants;
         static thread_local std::vector<EntityID> entities;
         registry.get_entities_with<CloudComponent>(entities);

         // Sort clouds by layer for correct depth ordering (Big < Medium < Small means Big renders first)
         std::sort(entities.begin(), entities.end(), [&](EntityID a, EntityID b) {
             auto ca_opt = registry.get_component<CloudComponent>(a);
             auto cb_opt = registry.get_component<CloudComponent>(b);
             if (!ca_opt || !cb_opt) return false;
             return static_cast<int>(ca_opt->get().layer) < static_cast<int>(cb_opt->get().layer);
         });

         sf::RenderWindow &window = renderer.window();
         const sf::View old_view = window.getView();
         window.setView(window.getDefaultView());

         for (auto entity: entities) {
             auto cloud_opt = registry.get_component<CloudComponent>(entity);
             if (!cloud_opt) continue;
             auto &cloud = cloud_opt->get();

             auto tex = assets.get_mutable_texture(cloud.texture_id);
             if (!tex) continue;

             // Create sprite and apply scale
             sf::Sprite sprite(*tex);
             sprite.setScale({cloud.scale, cloud.scale});

             // Position with parallax effect: clouds move less than camera
             float pos_x = cloud.x - camera.x() * CLOUD_PARALLAX;
             float pos_y = cloud.y;
             sprite.setPosition({pos_x, pos_y});
             window.draw(sprite);
         }

         window.setView(old_view);
     }
 } // namespace mario
