#pragma once

#include <unordered_map>
#include <typeindex>
#include <any>
#include <vector>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <optional>

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

    // Check if an entity has a specific component type
    template<typename T>
    bool has_component(EntityID id) const {
        auto type_it = _components.find(std::type_index(typeid(T)));
        if (type_it == _components.end()) return false;
        return type_it->second.find(id) != type_it->second.end();
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
            const auto& map = type_it->second;
            result.reserve(map.size());
            for (const auto& pair : map) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    // Non-allocating variant: reuse a caller-provided buffer
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

    // Multi-component query: returns entities that have ALL specified components
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

    // Three-component query: returns entities that have ALL specified components
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

    // Variadic multi-component query: returns entities that have ALL specified components.
    // Complexity: O(n * k) where n = size of the smallest component set, k = number of component types.
    // If any requested component type is missing from the registry, the result will be empty.
    template<typename... Ts>
    std::vector<EntityID> get_entities_with_all() const {
        std::vector<EntityID> result;
        get_entities_with_all<Ts...>(result);
        return result;
    }

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

    // Clears all components and resets entity counter.
    void clear() {
        _components.clear();
        _next_id = 0;
    }


private:
    EntityID _next_id = 0;
    std::unordered_map<std::type_index, std::unordered_map<EntityID, std::any>> _components;
};

} // namespace mario
