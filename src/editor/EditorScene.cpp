#include "Zia/editor/EditorScene.hpp"
#include "Zia/game/world/JsonHelper.hpp"

#include "Zia/engine/ecs/components/PositionComponent.hpp"
#include "Zia/engine/ecs/components/VelocityComponent.hpp"
#include "Zia/engine/ecs/components/SizeComponent.hpp"
#include "Zia/engine/ecs/components/SpriteComponent.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace zia::editor {

// Helper utilities for lightweight JSON-like parsing used by the project.
namespace {
    // Read entire stream to string
    std::string read_all(std::ifstream &in) {
        return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    }

    // Find an object block { ... } starting at or after 'start_pos'. Returns object content without braces or empty string.
    std::string extract_next_object(const std::string &text, std::size_t start_pos) {
        std::size_t obj_start = text.find('{', start_pos);
        if (obj_start == std::string::npos) return {};
        int depth = 0;
        for (std::size_t i = obj_start; i < text.size(); ++i) {
            if (text[i] == '{') ++depth;
            else if (text[i] == '}') --depth;
            if (depth == 0) {
                return text.substr(obj_start + 1, i - obj_start - 1);
            }
        }
        return {};
    }

    // Extract numeric pair from an array-like field: "key" : [a, b]
    bool extract_array2f(const std::string &text, std::string_view key, float &a, float &b) {
        const std::string needle = std::string("\"") + std::string(key) + "\"";
        auto pos = text.find(needle);
        if (pos == std::string::npos) return false;
        pos = text.find('[', pos);
        if (pos == std::string::npos) return false;
        ++pos;
        // parse first float
        try {
            // find comma
            auto comma = text.find(',', pos);
            if (comma == std::string::npos) return false;
            std::string first = text.substr(pos, comma - pos);
            // find closing bracket
            auto close = text.find(']', comma);
            if (close == std::string::npos) return false;
            std::string second = text.substr(comma + 1, close - comma - 1);
            a = std::stof(first);
            b = std::stof(second);
            return true;
        } catch (...) {
            return false;
        }
    }

    // Extract two separate numeric fields x and y (e.g. "x": 2, "y": 15)
    bool extract_xy_pair(const std::string &obj_text, std::string_view xkey, std::string_view ykey, float &x, float &y) {
        if (zia::JsonHelper::extract_float_field(obj_text, xkey, x) && zia::JsonHelper::extract_float_field(obj_text, ykey, y)) {
            return true;
        }
        return false;
    }

    // Trim whitespace
    std::string trim(const std::string &s) {
        auto l = s.find_first_not_of(" \t\n\r");
        if (l == std::string::npos) return {};
        auto r = s.find_last_not_of(" \t\n\r");
        return s.substr(l, r - l + 1);
    }
}

std::unique_ptr<ISceneEditor> create_editor_scene(zia::engine::IEntityManager& mgr) {
    return std::make_unique<EditorScene>(mgr);
}

EditorScene::EditorScene(zia::engine::IEntityManager& mgr) : _mgr(mgr) {}

bool EditorScene::open_scene(const std::string& path) {
    std::ifstream in = zia::JsonHelper::open_level_file(path);
    if (!in) return false;
    const std::string content = read_all(in);

    // Clear world before loading
    _mgr.clear();

    // Locate entities array
    std::size_t ent_pos = content.find("\"entities\"");
    if (ent_pos == std::string::npos) return true; // no entities, but success

    std::size_t array_start = content.find('[', ent_pos);
    if (array_start == std::string::npos) return true;

    std::size_t cur = array_start + 1;
    while (true) {
        // find next object
        std::size_t next_obj_start = content.find('{', cur);
        if (next_obj_start == std::string::npos) break;
        std::string obj = extract_next_object(content, next_obj_start);
        if (obj.empty()) break;

        // create entity
        zia::EntityID id = _mgr.create_entity();

        // Try various position representations: array 'position' or 'x'/'y'
        float px=0.0f, py=0.0f;
        if (extract_array2f(obj, "position", px, py) || extract_xy_pair(obj, "x", "y", px, py)) {
            _mgr.add_component<PositionComponent>(id, {px, py});
        }

        // Velocity: array or vx/vy
        float vx=0.0f, vy=0.0f;
        if (extract_array2f(obj, "velocity", vx, vy) || extract_xy_pair(obj, "vx", "vy", vx, vy)) {
            _mgr.add_component<VelocityComponent>(id, {vx, vy});
        }

        // Size: array 'size' or width/height
        float sw=0.0f, sh=0.0f;
        if (extract_array2f(obj, "size", sw, sh) || extract_xy_pair(obj, "width", "height", sw, sh)) {
            _mgr.add_component<SizeComponent>(id, {sw, sh});
        }

        // Sprite simple: texture_id or texture
        std::string tex;
        if (zia::JsonHelper::extract_string_field(obj, "texture", tex)) {
            SpriteComponent sc;
            // attempt to find numeric id after texture (not ideal) - default to -1
            sc.texture_id = -1;
            _mgr.add_component<SpriteComponent>(id, sc);
        }

        // move cursor past this object
        cur = content.find('}', next_obj_start);
        if (cur == std::string::npos) break;
        ++cur;
    }

    return true;
}

bool EditorScene::save_scene(const std::string& path) {
    std::ostringstream out;
    out << "{\n  \"entities\": [\n";

    std::vector<zia::EntityID> entities;
    _mgr.get_entities_with<PositionComponent>(entities);

    bool first = true;
    for (auto id : entities) {
        if (!first) out << ",\n";
        first = false;
        out << "    {\n";
        if (auto p = _mgr.get_component<PositionComponent>(id)) {
            out << "      \"position\": [" << p->get().x << ", " << p->get().y << "],\n";
        }
        if (auto v = _mgr.get_component<VelocityComponent>(id)) {
            out << "      \"velocity\": [" << v->get().vx << ", " << v->get().vy << "],\n";
        }
        if (auto s = _mgr.get_component<SizeComponent>(id)) {
            out << "      \"size\": [" << s->get().width << ", " << s->get().height << "],\n";
        }
        if (auto sc = _mgr.get_component<SpriteComponent>(id)) {
            out << "      \"sprite\": {\"texture_id\": " << sc->get().texture_id << "}\n";
        } else {
            // erase trailing comma of previous line if it exists
            out.seekp(-2, std::ios_base::cur);
            out << "\n";
        }
        out << "    }";
    }

    out << "\n  ]\n}\n";

    std::ofstream file(path);
    if (!file) return false;
    file << out.str();
    return true;
}

void EditorScene::play() {
    // TODO: implement play-in-editor by cloning the world and running simulation on the clone
}

void EditorScene::pause() {
    // No-op for the minimal implementation.
}

zia::EntityID EditorScene::spawn_entity() {
    return _mgr.create_entity();
}

void EditorScene::select_entity(zia::EntityID id) {
    _selected = id;
}

void EditorScene::undo() {}
void EditorScene::redo() {}

} // namespace zia::editor

