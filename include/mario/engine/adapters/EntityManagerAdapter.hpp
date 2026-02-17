#pragma once

#include "mario/engine/IEntityManager.hpp"
#include "mario/engine/ecs/EntityManager.hpp"
#include <memory>

namespace mario::engine::adapters {
    // Concrete implementation of IEntityManager that wraps the mario::EntityManager.
    class EntityManagerAdapter : public IEntityManager {
    public:
        explicit EntityManagerAdapter(std::shared_ptr<mario::EntityManager> e) : _entities(std::move(e)) {}
        ~EntityManagerAdapter() override = default;

        void clear() override { if (_entities) _entities->clear(); }
        mario::EntityID create_entity() override { return _entities ? _entities->create_entity() : 0; }

        mario::EntityManager& underlying() override { return *_entities; }
        const mario::EntityManager& underlying() const override { return *_entities; }

    private:
        std::shared_ptr<mario::EntityManager> _entities;
    };
} // namespace mario::engine::adapters

