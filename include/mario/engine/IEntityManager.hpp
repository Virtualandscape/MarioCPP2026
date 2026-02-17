#pragma once

#include "mario/ecs/EntityManager.hpp"

namespace mario::engine {
    class IEntityManager {
    public:
        virtual ~IEntityManager() = default;

        virtual void clear() = 0;
    };
} // namespace mario::engine

