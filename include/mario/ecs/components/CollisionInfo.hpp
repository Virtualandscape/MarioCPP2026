#pragma once

#include "mario/ecs/EntityType.hpp"

namespace mario {
    struct CollisionInfo {
        bool collided = false;
        EntityType other_type = EntityType::Unknown;
    };
} // namespace mario
