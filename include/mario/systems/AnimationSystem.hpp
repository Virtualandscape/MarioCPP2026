#pragma once

#include "mario/engine/IEntityManager.hpp"

namespace mario {
    class AnimationSystem {
    public:
        void update(mario::engine::IEntityManager& registry, float dt) const;
    };
} // namespace mario
