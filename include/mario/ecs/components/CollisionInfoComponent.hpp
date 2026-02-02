#pragma once

#include "mario/ecs/EntityTypeComponent.hpp"

namespace mario {
    struct CollisionInfoComponent {
        bool collided = false;
        EntityTypeComponent other_type = EntityTypeComponent::Unknown;
    };
} // namespace mario
