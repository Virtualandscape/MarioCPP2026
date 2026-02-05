#pragma once

#include "mario/render/Renderer.hpp"
#include "mario/ecs/EntityManager.hpp"

namespace mario {
    class Camera; // forward declaration

    // System responsible for drawing debug overlays such as bounding boxes.
    class DebugDrawSystem {
    public:
        DebugDrawSystem() = default;

        // Draw bounding boxes for entities that have Position and Size components.
        void render(Renderer& renderer, const Camera& camera, EntityManager& registry);
    };
}
