#pragma once

#include "mario/engine/ecs/EntityTypeComponent.hpp"

namespace mario {
    struct CollisionInfoComponent {
        bool collided = false;
        EntityTypeComponent other_type = EntityTypeComponent::Unknown;
    };
} // namespace mario
