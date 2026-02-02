#pragma once

#include "mario/ecs/EntityTypeComponent.hpp"

namespace mario {
    struct TypeComponent {
        EntityTypeComponent type = EntityTypeComponent::Unknown;
    };
} // namespace mario
