#include "mario/world/Level.hpp"
#include "mario/world/Camera.hpp"
#include "mario/world/TileMap.hpp"

namespace mario {
    void Level::load(std::string_view level_id) { (void) level_id; }

    void Level::unload() {
    }

    void Level::update(float dt) { (void) dt; }

    void Level::render() {
    }

    std::shared_ptr<TileMap> Level::tile_map() { return {}; }
    std::shared_ptr<Camera> Level::camera() { return {}; }
} // namespace mario
