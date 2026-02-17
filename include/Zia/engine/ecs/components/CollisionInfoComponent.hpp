#pragma once

#include "Zia/engine/ecs/EntityTypeComponent.hpp"

namespace zia {
    struct CollisionInfoComponent {
        bool collided = false;
        EntityTypeComponent other_type = EntityTypeComponent::Unknown;
    };
} // namespace Zia
