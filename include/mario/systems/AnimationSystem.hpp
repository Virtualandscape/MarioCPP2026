#pragma once

#include "mario/engine/EntityManagerFacade.hpp"

namespace mario {
    class AnimationSystem {
    public:
        void update(mario::engine::EntityManagerFacade& registry, float dt) const;
    };
} // namespace mario
