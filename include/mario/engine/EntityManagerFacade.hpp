#pragma once

#include "mario/ecs/EntityManager.hpp"

#include <vector>
#include <functional>
#include <optional>

namespace mario::engine {
    // A small facade over the concrete mario::EntityManager to avoid exposing the
    // project's template-heavy implementation to engine-consuming headers.
    // This class is non-owning and stores a reference to the underlying manager.
    class EntityManagerFacade {
    public:
        explicit EntityManagerFacade(mario::EntityManager &mgr) : _mgr(mgr) {}

        // Basic non-template operations
        void clear() { _mgr.clear(); }

        // Expose templated helpers as forwarding templates so callers can still use
        // the convenient typed API without including the concrete header.
        template<typename T>
        std::optional<std::reference_wrapper<T>> get_component(mario::EntityID id) {
            return _mgr.get_component<T>(id);
        }

        template<typename T>
        std::optional<std::reference_wrapper<const T>> get_component_const(mario::EntityID id) const {
            return _mgr.get_component<T>(id);
        }

        template<typename T>
        bool has_component(mario::EntityID id) const {
            return _mgr.has_component<T>(id);
        }

        template<typename T>
        void add_component(mario::EntityID id, T comp) {
            _mgr.add_component<T>(id, std::move(comp));
        }

        template<typename T>
        std::vector<mario::EntityID> get_entities_with() const {
            return _mgr.get_entities_with<T>();
        }

        template<typename T>
        void get_entities_with(std::vector<mario::EntityID>& out) const {
            _mgr.get_entities_with<T>(out);
        }

        // Variadic forwarding overload enabled only when at least two types are provided.
        template<typename... Ts, typename = std::enable_if_t<(sizeof...(Ts) >= 2)>>
        void get_entities_with(std::vector<mario::EntityID>& out) const {
            _mgr.get_entities_with<Ts...>(out);
        }

        // Forwarding helper to support queries for entities that have all listed components.
        template<typename... Ts>
        void get_entities_with_all(std::vector<mario::EntityID>& out) const {
            _mgr.get_entities_with_all<Ts...>(out);
        }

        template<typename T>
        void remove_component(mario::EntityID id) {
            _mgr.remove_component<T>(id);
        }

        // Create an entity via the underlying manager
        mario::EntityID create_entity() { return _mgr.create_entity(); }

    private:
        mario::EntityManager &_mgr;
    };
}
