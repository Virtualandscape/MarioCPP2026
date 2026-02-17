#pragma once

#include "mario/engine/IEntityManager.hpp"
#include "mario/ecs/EntityManager.hpp"

#include <memory>

namespace mario::engine::adapters {
    class EntityManagerAdapter : public IEntityManager {
    public:
        explicit EntityManagerAdapter(std::shared_ptr<mario::EntityManager> e) : _entities(std::move(e)) {}
        ~EntityManagerAdapter() override = default;

        void clear() override { if (_entities) _entities->clear(); }

        [[nodiscard]] std::shared_ptr<mario::EntityManager> underlying() const { return _entities; }

    private:
        std::shared_ptr<mario::EntityManager> _entities;
    };
} // namespace mario::engine::adapters

