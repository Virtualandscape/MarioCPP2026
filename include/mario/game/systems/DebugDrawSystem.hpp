#pragma once

#include "mario/engine/IEntityManager.hpp"
#include "mario/engine/IRenderer.hpp"

namespace mario {
    class Camera; // forward declaration

    // System responsible for drawing debug overlays such as bounding boxes.
    class DebugDrawSystem {
    public:
        DebugDrawSystem() = default;

        // Draw bounding boxes for entities that have Position and Size components.
        void render(mario::engine::IRenderer& renderer, const Camera& camera, mario::engine::IEntityManager& registry);
    };
}
