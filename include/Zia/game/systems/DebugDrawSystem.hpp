#pragma once

#include "Zia/engine/IEntityManager.hpp"
#include "Zia/engine/IRenderer.hpp"

namespace zia {
    class Camera; // forward declaration

    // System responsible for drawing debug overlays such as bounding boxes.
    class DebugDrawSystem {
    public:
        DebugDrawSystem() = default;

        // Draw bounding boxes for entities that have Position and Size components.
        void render(zia::engine::IRenderer& renderer, const Camera& camera, zia::engine::IEntityManager& registry);
    };
}
