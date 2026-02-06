#pragma once

#include "mario/ecs/EntityManager.hpp"

namespace mario {
    class AnimationSystem {
    public:
        void update(EntityManager& registry, float dt) const;
    };
} // namespace mario
