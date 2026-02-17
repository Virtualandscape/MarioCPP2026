#pragma once

#include "mario/engine/ecs/EntityTypeComponent.hpp"

namespace mario {
    struct TypeComponent {
        EntityTypeComponent type = EntityTypeComponent::Unknown;
    };
} // namespace mario
