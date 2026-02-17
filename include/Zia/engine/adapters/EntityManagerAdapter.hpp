#pragma once

#include "Zia/engine/IEntityManager.hpp"
#include "Zia/engine/ecs/EntityManager.hpp"
#include <memory>

namespace zia::engine::adapters {
    // Concrete implementation of IEntityManager that wraps the Zia::EntityManager.
    class EntityManagerAdapter : public IEntityManager {
    public:
        explicit EntityManagerAdapter(std::shared_ptr<zia::EntityManager> e) : _entities(std::move(e)) {}
        ~EntityManagerAdapter() override = default;

        void clear() override { if (_entities) _entities->clear(); }
        zia::EntityID create_entity() override { return _entities ? _entities->create_entity() : 0; }

        zia::EntityManager& underlying() override { return *_entities; }
        const zia::EntityManager& underlying() const override { return *_entities; }

    private:
        std::shared_ptr<zia::EntityManager> _entities;
    };
} // namespace Zia::engine::adapters

