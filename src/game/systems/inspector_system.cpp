#include "Zia/game/systems/InspectorSystem.hpp"
#include "Zia/engine/ecs/components/PositionComponent.hpp"
#include "Zia/engine/ecs/components/VelocityComponent.hpp"
#include "Zia/engine/ecs/components/SpriteComponent.hpp"
#include "Zia/engine/ecs/components/TypeComponent.hpp"
#include "Zia/engine/ecs/components/AnimationComponent.hpp"
#include "Zia/engine/ecs/components/EnemyComponent.hpp"
#include "Zia/engine/ecs/components/SizeComponent.hpp"
#include "Zia/engine/resources/AssetManager.hpp"
#include "Zia/game/helpers/Constants.hpp"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>

namespace zia {

InspectorSystem::InspectorSystem() {
    _enabled = true;
    _max_entries = 32;
}

void InspectorSystem::update(zia::engine::IEntityManager& /*registry*/, float /*dt*/) {
    // No internal state for now.
}

// Helper: build a list of human-readable lines for the provided entities
void InspectorSystem::build_lines(const std::vector<EntityID>& entities, zia::engine::IEntityManager& registry, std::vector<std::string>& out_lines, zia::engine::IAssetManager& /*assets*/) const {
    // registry, entities and out_lines are used below

    out_lines.clear();
    std::ostringstream header;
    header << "Inspector - entities: " << entities.size();
    out_lines.push_back(header.str());

    size_t count = 0;
    for (auto entity : entities) {
        if (count++ >= _max_entries) break;
        std::ostringstream oss;
        oss << "Entity " << entity << ": ";
        // Type
        if (auto type_opt = registry.get_component<TypeComponent>(entity)) {
            auto& t = type_opt->get();
            switch (t.type) {
                case EntityTypeComponent::Player: oss << "Player"; break;
                case EntityTypeComponent::Goomba: oss << "Goomba"; break;
                case EntityTypeComponent::Koopa: oss << "Koopa"; break;
                default: oss << "TypeUnknown"; break;
            }
        } else if (registry.get_component<EnemyComponent>(entity)) {
            oss << "Enemy";
        } else {
            oss << "Entity";
        }

        // Position
        if (auto pos_opt = registry.get_component<PositionComponent>(entity)) {
            auto& p = pos_opt->get();
            oss << " pos=(" << std::fixed << std::setprecision(1) << p.x << "," << p.y << ")";
        }
        // Velocity
        if (auto v_opt = registry.get_component<VelocityComponent>(entity)) {
            auto& v = v_opt->get();
            oss << " vel=(" << std::fixed << std::setprecision(1) << v.vx << "," << v.vy << ")";
        }
        // Size
        if (auto s_opt = registry.get_component<SizeComponent>(entity)) {
            auto& s = s_opt->get();
            oss << " size=(" << std::fixed << std::setprecision(1) << s.width << "," << s.height << ")";
        }
        // Sprite
        if (auto sp_opt = registry.get_component<SpriteComponent>(entity)) {
            auto& sp = sp_opt->get();
            if (sp.texture_id >= 0) {
                oss << " sprite_id=" << sp.texture_id;
            }
        }
        // Animation state
        if (auto a_opt = registry.get_component<AnimationComponent>(entity)) {
            auto& a = a_opt->get();
            switch (a.current_state) {
                case AnimationComponent::State::Idle: oss << " anim=Idle"; break;
                case AnimationComponent::State::Run: oss << " anim=Run"; break;
                case AnimationComponent::State::Jump: oss << " anim=Jump"; break;
                default: break;
            }
        }

        out_lines.push_back(oss.str());
    }
}

void InspectorSystem::render_ui(zia::engine::IEntityManager& registry, zia::engine::IAssetManager& assets) {
    if (!_enabled) return;

    // Collect candidate entities: entities with TypeComponent and EnemyComponent.
    static thread_local std::vector<EntityID> type_entities;
    static thread_local std::vector<EntityID> enemy_entities;
    type_entities.clear();
    enemy_entities.clear();

    registry.get_entities_with<TypeComponent>(type_entities);
    registry.get_entities_with<EnemyComponent>(enemy_entities);

    // Merge lists, keeping order and uniqueness.
    std::vector<EntityID> entities;
    entities.reserve(type_entities.size() + enemy_entities.size());
    for (auto e: type_entities) entities.push_back(e);
    for (auto e: enemy_entities) {
        if (std::find(entities.begin(), entities.end(), e) == entities.end()) entities.push_back(e);
    }

    // If no candidate entities found, early out.
    if (entities.empty()) return;

    // Build display lines
    std::vector<std::string> lines;
    build_lines(entities, registry, lines, assets);

    ImGui::SetNextWindowPos(ImVec2(8.0f, 8.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Inspector", &_enabled)) {
        for (const auto &line : lines) {
            ImGui::TextUnformatted(line.c_str());
        }
    }
    ImGui::End();
}

} // namespace Zia
