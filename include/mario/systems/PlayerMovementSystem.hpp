#pragma once

#include "mario/ecs/Registry.hpp"

namespace mario {

    class PlayerMovementSystem {
    public:
        void update(Registry& registry, float dt) const;
    };
} // namespace mario
