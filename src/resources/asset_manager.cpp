#include "mario/resources/AssetManager.hpp"

namespace mario {

void AssetManager::load_texture(int id, std::string_view path)
{
    (void)id;
    (void)path;
}
void AssetManager::load_sound(int id, std::string_view path)
{
    (void)id;
    (void)path;
}
void AssetManager::unload_all() {}

} // namespace mario
