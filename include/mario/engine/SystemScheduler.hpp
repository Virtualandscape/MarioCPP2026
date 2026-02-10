#pragma once

#include "mario/ecs/EntityManager.hpp"
#include "mario/render/Renderer.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/world/Camera.hpp"

#include <functional>
#include <vector>

namespace mario::engine {

    // Simple scheduler to run ordered ECS systems inside the engine loop.
    class SystemScheduler {
    public:
        using UpdateFn = std::function<void(EntityManager&, float)>;
        using RenderFn = std::function<void(EntityManager&, Renderer&, AssetManager&, const Camera&)>;

        void add_system(UpdateFn system);
        void add_render_system(RenderFn system);

        void update(EntityManager& registry, float dt);
        void render(EntityManager& registry, Renderer& renderer, AssetManager& assets, const Camera& camera);

        void clear();

    private:
        std::vector<UpdateFn> _update_systems;
        std::vector<RenderFn> _render_systems;
    };

}
