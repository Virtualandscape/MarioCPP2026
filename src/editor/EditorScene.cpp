#include "Zia/editor/EditorScene.hpp"
#include "Zia/game/world/JsonHelper.hpp"
#include "Zia/game/helpers/Spawner.hpp"
#include "Zia/game/world/EntitySpawn.hpp"

#include "Zia/engine/ecs/components/PositionComponent.hpp"
#include "Zia/engine/ecs/components/ColorComponent.hpp"
#include "Zia/engine/ecs/components/NameComponent.hpp"
#include "Zia/game/helpers/Constants.hpp"

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

std::unique_ptr<ISceneEditor> create_editor_scene(zia::engine::IEntityManager& mgr, zia::engine::IAssetManager& assets) {
    return std::make_unique<EditorScene>(mgr, assets);
}

EditorScene::EditorScene(zia::engine::IEntityManager& mgr, zia::engine::IAssetManager& assets) : _mgr(mgr), _assets(assets) {}

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
        zia::EntityID id = 0;

        // Try to detect type first
        std::string type_str;
        if (zia::JsonHelper::extract_string_field(obj, "type", type_str)) {
            std::transform(type_str.begin(), type_str.end(), type_str.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        }

        // If it's a player spawn, prefer to use Spawner to create a proper entity composition at tile coords.
        if (type_str == "player") {
            int tile_x = 0, tile_y = 0;
            // try extract tile coordinates (use existing float extract helper and cast)
            float fx=0.0f, fy=0.0f;
            if (extract_array2f(obj, "position", fx, fy) || extract_xy_pair(obj, "x", "y", fx, fy)) {
                tile_x = static_cast<int>(std::lround(fx));
                tile_y = static_cast<int>(std::lround(fy));
            }
            // optional name
            std::string name;
            zia::JsonHelper::extract_string_field(obj, "name", name);
            zia::EntitySpawn spawn;
            spawn.type = "player";
            spawn.tile_x = tile_x;
            spawn.tile_y = tile_y;
            id = Spawner::spawn_player(_mgr, spawn, _assets);
            if (!name.empty()) {
                _mgr.add_component<NameComponent>(id, {name});
            }
        } else {
            id = _mgr.create_entity();

            // Try various position representations: array 'position' or 'x'/'y'
            float px=0.0f, py=0.0f;
            if (extract_array2f(obj, "position", px, py) || extract_xy_pair(obj, "x", "y", px, py)) {
                const float tile_size = static_cast<float>(zia::constants::TILE_SIZE);
                float world_x = px * tile_size;
                float world_y = py * tile_size;
                _mgr.add_component<PositionComponent>(id, {world_x, world_y});
            }

            // Optional name field
            std::string name;
            if (zia::JsonHelper::extract_string_field(obj, "name", name)) {
                _mgr.add_component<NameComponent>(id, {name});
            }

            // Optional color field: expect array [r,g,b,a] in 0..1 range
            float cr=1.0f, cg=1.0f, cb=1.0f, ca=1.0f;
            if (extract_array2f(obj, "color", cr, cg)) {
                // crude parse: extract_array2f reads two floats; try to read third and fourth manually
                // look for the 'color' array again and parse up to 4 elements
                const std::string needle = std::string("\"color\"");
                auto pos = obj.find(needle);
                if (pos != std::string::npos) {
                    pos = obj.find('[', pos);
                    if (pos != std::string::npos) {
                        ++pos;
                        try {
                            // find commas and closing bracket
                            auto comma1 = obj.find(',', pos);
                            auto comma2 = obj.find(',', comma1 + 1);
                            auto comma3 = obj.find(',', comma2 + 1);
                            auto close = obj.find(']', pos);
                            std::string s1 = obj.substr(pos, comma1 - pos);
                            std::string s2 = obj.substr(comma1 + 1, (comma2==std::string::npos?close:comma2) - comma1 - 1);
                            cr = std::stof(s1);
                            cg = std::stof(s2);
                            if (comma2 != std::string::npos) {
                                std::string s3 = obj.substr(comma2 + 1, (comma3==std::string::npos?close:comma3) - comma2 - 1);
                                cb = std::stof(s3);
                            }
                            if (comma3 != std::string::npos) {
                                std::string s4 = obj.substr(comma3 + 1, close - comma3 - 1);
                                ca = std::stof(s4);
                            }
                        } catch (...) {}
                    }
                }
                _mgr.add_component<ColorComponent>(id, {cr, cg, cb, ca});
            }
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
        // Write name if present (as a top-level string field)
        if (auto n = _mgr.get_component<NameComponent>(id)) {
            out << "      \"name\": \"" << n->get().value << "\"," << "\n";
        }
        if (auto p = _mgr.get_component<PositionComponent>(id)) {
            out << "      \"position\": [" << p->get().x << ", " << p->get().y << "],\n";
        }
        if (auto c = _mgr.get_component<ColorComponent>(id)) {
            out << "      \"color\": [" << c->get().r << ", " << c->get().g << ", " << c->get().b << ", " << c->get().a << "],\n";
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

