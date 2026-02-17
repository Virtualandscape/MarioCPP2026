#pragma once

#include <unordered_map>
#include <typeindex>
#include <any>
#include <vector>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <optional>

namespace zia {

// Type alias for entity identifiers.
using EntityID = uint32_t;

// Manages entities and their components in a simple type-indexed registry.
// Components are stored as std::any keyed by std::type_index and by EntityID.
class EntityManager {
public:
    // Used by: Spawner::spawn_* functions, various systems when creating entities (background, clouds, enemies)
    // Create a new entity and return its unique ID.
    // IDs start from 1 and increment; 0 is reserved as an invalid ID.
    EntityID create_entity() {
        return ++_next_id;
    }

    // Used by: Spawner (adds components for spawned entities)
    // Add a component of type T to the given entity ID by value.
    // Overwrites any existing component of the same type for that entity.
    template<typename T>
    void add_component(EntityID id, T comp) {
        _components[std::type_index(typeid(T))][id] = comp;
    }

    // Used by: Systems and helpers that need mutable access (e.g. PhysicsSystem, EnemySystem, AnimationSystem, SpriteRenderSystem)
    // Retrieve a mutable reference to a component of type T for an entity.
    // Returns std::nullopt if the component or entity is not present or if the any_cast fails.
    template<typename T>
    std::optional<std::reference_wrapper<T>> get_component(EntityID id) {
        auto type_it = _components.find(std::type_index(typeid(T)));
        if (type_it == _components.end()) return std::nullopt;
        auto& map = type_it->second;
        auto it = map.find(id);
        if (it == map.end()) return std::nullopt;
        try {
            return std::optional<std::reference_wrapper<T>>(std::ref(std::any_cast<T&>(it->second)));
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    // Used by: Systems that only need read-only access (various systems called in rendering or logic checks)
    // Const-qualified variant: retrieve a const reference to a component of type T.
    template<typename T>
    std::optional<std::reference_wrapper<const T>> get_component(EntityID id) const {
        auto type_it = _components.find(std::type_index(typeid(T)));
        if (type_it == _components.end()) return std::nullopt;
        const auto& map = type_it->second;
        auto it = map.find(id);
        if (it == map.end()) return std::nullopt;
        try {
            return std::optional<std::reference_wrapper<const T>>(std::cref(std::any_cast<const T&>(it->second)));
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    // Used by: Logic that tests presence before operating (collision checks, system guards)
    // Check whether an entity has a component of type T.
    template<typename T>
    bool has_component(EntityID id) const {
        auto type_it = _components.find(std::type_index(typeid(T)));
        if (type_it == _components.end()) return false;
        return type_it->second.find(id) != type_it->second.end();
    }

    // Used by: Cleanup paths or when removing dynamic components at runtime (few callsites)
    // Remove a component of type T from an entity (no-op if missing).
    template<typename T>
    void remove_component(EntityID id) {
        auto type_it = _components.find(std::type_index(typeid(T)));
        if (type_it != _components.end()) {
            type_it->second.erase(id);
        }
    }

    // Used by: Systems that iterate entities with a single component (CollisionSystem, CloudSystem, etc.)
    // Collect all entities that have component T. Allocating variant that returns a vector.
    template<typename T>
    std::vector<EntityID> get_entities_with() const {
        std::vector<EntityID> result;
        auto type_it = _components.find(std::type_index(typeid(T)));
        if (type_it != _components.end()) {
            const auto& map = type_it->second;
            result.reserve(map.size());
            for (const auto& pair : map) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    // Used by: Systems that prefer to reuse buffers to avoid allocations (sprite render, physics, debug draw)
    // Non-allocating variant: fills the caller-provided vector.
    template<typename T>
    void get_entities_with(std::vector<EntityID>& out) const {
        out.clear();
        auto type_it = _components.find(std::type_index(typeid(T)));
        if (type_it != _components.end()) {
            const auto& map = type_it->second;
            out.reserve(map.size());
            for (const auto& pair : map) {
                out.push_back(pair.first);
            }
        }
    }

    // Used by: SpriteRenderSystem, CollisionSystem and other systems that need entities with multiple components
    // Query for entities that have BOTH component types T1 and T2.
    // Efficiently iterates the smaller of the two component maps.
    template<typename T1, typename T2>
    void get_entities_with(std::vector<EntityID>& out) const {
        out.clear();
        auto type_it = _components.find(std::type_index(typeid(T1)));
        if (type_it == _components.end()) return;

        const auto& map1 = type_it->second;
        const auto& map2_it = _components.find(std::type_index(typeid(T2)));
        if (map2_it == _components.end()) return;
        const auto& map2 = map2_it->second;

        // Iterate the smaller map for efficiency
        if (map1.size() <= map2.size()) {
            out.reserve(map1.size());
            for (const auto& pair : map1) {
                if (map2.find(pair.first) != map2.end()) {
                    out.push_back(pair.first);
                }
            }
        } else {
            out.reserve(map2.size());
            for (const auto& pair : map2) {
                if (map1.find(pair.first) != map1.end()) {
                    out.push_back(pair.first);
                }
            }
        }
    }

    // Used by: SpriteRenderSystem, DebugDrawSystem and AnimationSystem for triple-component queries
    // Query for entities that have T1, T2 and T3. Iterates the smallest map for performance.
    template<typename T1, typename T2, typename T3>
    void get_entities_with(std::vector<EntityID>& out) const {
        out.clear();
        auto type_it1 = _components.find(std::type_index(typeid(T1)));
        auto type_it2 = _components.find(std::type_index(typeid(T2)));
        auto type_it3 = _components.find(std::type_index(typeid(T3)));
        if (type_it1 == _components.end() || type_it2 == _components.end() || type_it3 == _components.end()) {
            return;
        }

        const auto& map1 = type_it1->second;
        const auto& map2 = type_it2->second;
        const auto& map3 = type_it3->second;

        // Find the smallest map and iterate it for efficiency
        const auto& smallest_map = (map1.size() <= map2.size() && map1.size() <= map3.size()) ? map1
                                  : (map2.size() <= map3.size()) ? map2 : map3;

        out.reserve(smallest_map.size());
        for (const auto& pair : smallest_map) {
            if (map1.find(pair.first) != map1.end() &&
                map2.find(pair.first) != map2.end() &&
                map3.find(pair.first) != map3.end()) {
                out.push_back(pair.first);
            }
        }
    }

    // Used by: PlayerControllerSystem, AnimationSystem — common-case convenience for two types
    // Variadic query: returns entities that have ALL specified component types.
    // Uses a non-allocating overload that fills the provided vector.
    // Complexity: O(n * k) where n = size of the smallest component set, k = number of types.
    template<typename... Ts>
    std::vector<EntityID> get_entities_with_all() const {
        std::vector<EntityID> result;
        get_entities_with_all<Ts...>(result);
        return result;
    }

    // Used by: Various systems that need to query arbitrary component sets without causing allocations
    // Fills 'out' with entities that have all component types in Ts... .
    template<typename... Ts>
    void get_entities_with_all(std::vector<EntityID>& out) const {
        static_assert(sizeof...(Ts) > 0, "get_entities_with_all requires at least one component type");
        out.clear();

        // Collect pointers to the component maps for each requested type.
        std::vector<const std::unordered_map<EntityID, std::any>*> maps;
        maps.reserve(sizeof...(Ts));

        bool missing = false;
        // Expand the parameter pack and push map pointers; if any component type is missing, mark missing.
        (void)std::initializer_list<int>{ ([&]() {
            auto it = _components.find(std::type_index(typeid(Ts)));
            if (it == _components.end()) {
                missing = true;
            } else {
                maps.push_back(&it->second);
            }
            return 0;
        }(), 0)... };

        if (missing) return;

        // Choose the smallest map to iterate for efficiency
        size_t min_idx = 0;
        for (size_t i = 1; i < maps.size(); ++i) {
            if (maps[i]->size() < maps[min_idx]->size()) min_idx = i;
        }

        out.reserve(maps[min_idx]->size());

        for (const auto& pair : *maps[min_idx]) {
            const EntityID id = pair.first;
            bool present_in_all = true;
            for (size_t i = 0; i < maps.size(); ++i) {
                if (i == min_idx) continue;
                if (maps[i]->find(id) == maps[i]->end()) {
                    present_in_all = false;
                    break;
                }
            }
            if (present_in_all) out.push_back(id);
        }
    }

    // Used by: Test/cleanup code and when resetting the ECS between levels
    // Clear all components and reset the entity counter to zero.
    void clear() {
        _components.clear();
        _next_id = 0;
    }


private:
    // Next entity ID to assign. Starts at 0; first entity will have ID 1.
    EntityID _next_id = 0;
    // Map from component type index -> map(entity id -> component stored as std::any).
    std::unordered_map<std::type_index, std::unordered_map<EntityID, std::any>> _components;
};

} // namespace Zia
