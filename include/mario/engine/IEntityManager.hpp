#pragma once

#include "mario/engine/ecs/EntityManager.hpp"
#include <vector>
#include <functional>
#include <optional>

namespace mario::engine {
    // Consolidated Entity Management interface.
    // Provides a bridge to the concrete EntityManager while maintaining an engine-agnostic interface.
    class IEntityManager {
    public:
        virtual ~IEntityManager() = default;

        // Basic non-template operations that can be polymorphic.
        virtual void clear() = 0;
        virtual mario::EntityID create_entity() = 0;

        // Bridge to the underlying concrete manager for template operations.
        virtual mario::EntityManager& underlying() = 0;
        virtual const mario::EntityManager& underlying() const = 0;

        // Templated helpers forwarding to the underlying manager.
        // These are non-virtual but use the virtual underlying() to reach the concrete implementation.
        template<typename T>
        std::optional<std::reference_wrapper<T>> get_component(mario::EntityID id) {
            return underlying().get_component<T>(id);
        }

        template<typename T>
        std::optional<std::reference_wrapper<const T>> get_component_const(mario::EntityID id) const {
            return underlying().get_component<T>(id);
        }

        template<typename T>
        bool has_component(mario::EntityID id) const {
            return underlying().has_component<T>(id);
        }

        template<typename T>
        void add_component(mario::EntityID id, T comp) {
            underlying().add_component<T>(id, std::move(comp));
        }

        template<typename T>
        std::vector<mario::EntityID> get_entities_with() const {
            return underlying().get_entities_with<T>();
        }

        template<typename T>
        void get_entities_with(std::vector<mario::EntityID>& out) const {
            underlying().get_entities_with<T>(out);
        }

        // Variadic forwarding overload enabled only when at least two types are provided.
        template<typename... Ts, typename = std::enable_if_t<(sizeof...(Ts) >= 2)>>
        void get_entities_with(std::vector<mario::EntityID>& out) const {
            underlying().get_entities_with<Ts...>(out);
        }

        // Forwarding helper to support queries for entities that have all listed components.
        template<typename... Ts>
        void get_entities_with_all(std::vector<mario::EntityID>& out) const {
            underlying().get_entities_with_all<Ts...>(out);
        }

        template<typename T>
        void remove_component(mario::EntityID id) {
            underlying().remove_component<T>(id);
        }
    };
}

