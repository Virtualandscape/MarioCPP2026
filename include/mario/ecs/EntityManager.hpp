#pragma once

#include <unordered_map>
#include <typeindex>
#include <any>
#include <vector>
#include <cstdint>

namespace mario {

using EntityID = uint32_t;

class EntityManager {
public:
    EntityID create_entity() {
        return ++_next_id;
    }

    template<typename T>
    void add_component(EntityID id, T comp) {
        _components[std::type_index(typeid(T))][id] = comp;
    }

    template<typename T>
    T* get_component(EntityID id) {
        auto type_it = _components.find(std::type_index(typeid(T)));
        if (type_it == _components.end()) return nullptr;
        auto& map = type_it->second;
        auto it = map.find(id);
        if (it == map.end()) return nullptr;
        return &std::any_cast<T&>(it->second);
    }

    template<typename T>
    const T* get_component(EntityID id) const {
        auto type_it = _components.find(std::type_index(typeid(T)));
        if (type_it == _components.end()) return nullptr;
        auto& map = type_it->second;
        auto it = map.find(id);
        if (it == map.end()) return nullptr;
        return &std::any_cast<const T&>(it->second);
    }

    template<typename T>
    void remove_component(EntityID id) {
        auto type_it = _components.find(std::type_index(typeid(T)));
        if (type_it != _components.end()) {
            type_it->second.erase(id);
        }
    }

    // Simple iteration: get all entities that have a certain component
    template<typename T>
    std::vector<EntityID> get_entities_with() const {
        std::vector<EntityID> result;
        auto type_it = _components.find(std::type_index(typeid(T)));
        if (type_it != _components.end()) {
            for (auto& pair : type_it->second) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    // Clears all components and resets entity counter.
    void clear() {
        _components.clear();
        _next_id = 0;
    }

    // For multiple components, can add later if needed

private:
    EntityID _next_id = 0;
    std::unordered_map<std::type_index, std::unordered_map<EntityID, std::any>> _components;
};

} // namespace mario
