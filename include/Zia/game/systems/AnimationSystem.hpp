#pragma once

#include "Zia/engine/IEntityManager.hpp"

namespace zia {
    class AnimationSystem {
    public:
        void update(zia::engine::IEntityManager& registry, float dt) const;
    };
} // namespace Zia
