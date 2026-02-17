#include "mario/systems/InspectorSystem.hpp"
#include "mario/ecs/components/PositionComponent.hpp"
#include "mario/ecs/components/VelocityComponent.hpp"
#include "mario/ecs/components/SpriteComponent.hpp"
#include "mario/ecs/components/TypeComponent.hpp"
#include "mario/ecs/components/AnimationComponent.hpp"
#include "mario/ecs/components/EnemyComponent.hpp"
#include "mario/ecs/components/SizeComponent.hpp"
#include "mario/resources/AssetManager.hpp"
#include "mario/helpers/Constants.hpp"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>

namespace mario {

InspectorSystem::InspectorSystem()
    : _fallback_font(), _text(_fallback_font) {
    _enabled = true;
    _max_entries = 32;
}

void InspectorSystem::initialize(mario::engine::IAssetManager& assets) {
    // Prefer AssetManager-managed font
    const int font_id = constants::FONT_MONTSERRAT_BLACK_ID;
    bool loaded = false;

    if (assets.has_font(font_id)) {
        _font_ptr = assets.get_font(font_id);
        if (_font_ptr) {
            loaded = true;
        }
    } else {
        if (assets.load_font(font_id, "assets/fonts/Montserrat-Black.ttf")) {
            _font_ptr = assets.get_font(font_id);
            if (_font_ptr) {
                loaded = true;
            }
        }
    }

    if (!loaded) {
        // Fallback to direct file loading using local resolution (try several relative paths)
        const std::vector<std::string> tries = {
            "assets/fonts/Montserrat-Black.ttf",
            "../assets/fonts/Montserrat-Black.ttf",
            "../../assets/fonts/Montserrat-Black.ttf",
        };
        for (const auto &p : tries) {
            try {
                if (std::filesystem::exists(p)) {
                    if (_fallback_font.openFromFile(p)) { loaded = true; break; }
                }
            } catch (const std::filesystem::filesystem_error&) {
                // ignore path errors and continue
            }
        }
        if (loaded) {
            // make fallback font visible via _text
            _text.setFont(_fallback_font);
        }
    } else {
        // Use the shared font from AssetManager without copying
        if (_font_ptr) _text.setFont(*_font_ptr);
    }

    // common text settings
    _text.setCharacterSize(14);
    _text.setFillColor(sf::Color::White);
}

void InspectorSystem::update(mario::engine::EntityManagerFacade& /*registry*/, float /*dt*/) {
    // No internal state for now.
}

// Helper: build a list of human-readable lines for the provided entities
void InspectorSystem::build_lines(const std::vector<EntityID>& entities, mario::engine::EntityManagerFacade& registry, std::vector<std::string>& out_lines, mario::engine::IAssetManager& /*assets*/) const {
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

void InspectorSystem::render(mario::engine::IRenderer& renderer, const Camera& /*camera*/, mario::engine::EntityManagerFacade& registry, mario::engine::IAssetManager& assets) {
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

    // Draw lines in UI space; prefer using Renderer::draw_text which uses renderer's font.
    float y = 8.0f;
    const float x = 8.0f;
    const unsigned int size = 14;

    // Draw using the configured font (AssetManager font or fallback)
    sf::RenderWindow &window = renderer.window();
    auto old_view = window.getView();
    window.setView(window.getDefaultView());

    for (const auto &line: lines) {
        _text.setString(line);
        _text.setPosition({x, y});
        window.draw(_text);
        y += static_cast<float>(size) + 2.0f;
    }

    window.setView(old_view);
}

} // namespace mario
