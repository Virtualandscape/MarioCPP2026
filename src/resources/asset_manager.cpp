#include "mario/resources/AssetManager.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace mario {

    static std::ifstream open_file_multi(std::string_view path) {
        std::filesystem::path base(path);
        std::ifstream file{base.string()};
        if (file) return file;

        const std::filesystem::path cwd = std::filesystem::current_path();
        const std::filesystem::path tries[] = {
            cwd / base,
            cwd / ".." / base,
            cwd / ".." / ".." / base,
            cwd / ".." / ".." / ".." / base,
        };

        for (const auto &candidate: tries) {
            file = std::ifstream{candidate.string()};
            if (file) return file;
        }

        return {};
    }

    bool AssetManager::load_texture(int id, std::string_view path) {
        if (path.empty()) return false;
        if (has_texture(id)) return true;

        // Try opening the file first (to search relative paths similarly to other loaders)
        std::ifstream file = open_file_multi(path);
        if (!file) {
            std::cerr << "AssetManager: failed to open texture file '" << std::string(path) << "'\n";
            return false;
        }

        // Use SFML texture loading directly from path (sf::Texture::loadFromFile expects a path)
        auto tex = std::make_shared<sf::Texture>();
        if (!tex->loadFromFile(std::string(path))) {
            // If direct load failed, try to load from the actual found path
            file.clear();
            file.seekg(0);
            std::filesystem::path actual;
            // Re-run open_file_multi to discover which candidate exists
            std::filesystem::path base(path);
            const std::filesystem::path cwd = std::filesystem::current_path();
            const std::filesystem::path tries[] = {
                base,
                cwd / base,
                cwd / ".." / base,
                cwd / ".." / ".." / base,
                cwd / ".." / ".." / ".." / base,
            };
            bool loaded = false;
            for (const auto &candidate : tries) {
                if (std::filesystem::exists(candidate)) {
                    if (tex->loadFromFile(candidate.string())) {
                        loaded = true;
                        break;
                    }
                }
            }
            if (!loaded) {
                std::cerr << "AssetManager: failed to load texture '" << std::string(path) << "' as image\n";
                return false;
            }
        }

        // Enable smoothing so scaled-up backgrounds don't look blocky
        tex->setSmooth(true);

        _textures[id] = tex;
        return true;
    }

    sf::Texture* AssetManager::get_mutable_texture(int id) {
        auto it = _textures.find(id);
        if (it == _textures.end()) return nullptr;
        return it->second.get();
    }

    const sf::Texture* AssetManager::get_texture(int id) const {
        auto it = _textures.find(id);
        if (it == _textures.end()) return nullptr;
        return it->second.get();
    }

    bool AssetManager::has_texture(int id) const {
        return _textures.find(id) != _textures.end();
    }

    void AssetManager::load_sound(int id, std::string_view path) {
        (void)id;
        (void)path;
    }

    void AssetManager::unload_all() {
        _textures.clear();
    }

} // namespace mario
