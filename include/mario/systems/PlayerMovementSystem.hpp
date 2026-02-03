#pragma once

#include "mario/ecs/EntityManager.hpp"

namespace mario {

    class PlayerMovementSystem {
    public:
        void update(EntityManager& registry, float dt) const;
    };
} // namespace mario
