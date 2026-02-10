#include "mario/engine/SystemScheduler.hpp"

namespace mario {
namespace engine {

void SystemScheduler::add_system(UpdateFn system) {
    if (system) {
        _update_systems.push_back(std::move(system));
    }
}

void SystemScheduler::add_render_system(RenderFn system) {
    if (system) {
        _render_systems.push_back(std::move(system));
    }
}

void SystemScheduler::update(EntityManager& registry, float dt) {
    for (auto& system : _update_systems) {
        system(registry, dt);
    }
}

void SystemScheduler::render(EntityManager& registry, Renderer& renderer, AssetManager& assets, const Camera& camera) {
    for (auto& render_system : _render_systems) {
        render_system(registry, renderer, assets, camera);
    }
}

void SystemScheduler::clear() {
    _update_systems.clear();
    _render_systems.clear();
}

} // namespace engine
} // namespace mario

