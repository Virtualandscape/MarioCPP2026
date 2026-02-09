// Implements the AssetManager class, which loads and manages textures and other assets for the game.
// Handles loading textures from disk, searching multiple locations, and reporting errors if assets are missing.

#include "mario/resources/AssetManager.hpp"

#include <filesystem>
#include <iostream>
#include <optional>

namespace mario {

    static std::optional<std::filesystem::path> resolve_asset_path(std::string_view path) {
        std::filesystem::path base(path);

        const std::filesystem::path cwd = std::filesystem::current_path();
        const std::filesystem::path tries[] = {
            base,
            cwd / base,
            cwd / ".." / base,
            cwd / ".." / ".." / base,
            cwd / ".." / ".." / ".." / base,
        };

        for (const auto& candidate : tries) {
            if (std::filesystem::exists(candidate)) {
                return candidate;
            }
        }

        return std::nullopt;
    }

    // Loads a texture from disk and stores it with the given ID. Returns true on success.
    bool AssetManager::load_texture(int id, std::string_view path) {
        if (path.empty()) return false;
        if (has_texture(id)) return true;

        const auto resolved_path = resolve_asset_path(path);
        if (!resolved_path) {
            std::cerr << "AssetManager: failed to find texture file '" << std::string(path) << "'\n";
            return false;
        }

        auto tex = std::make_shared<sf::Texture>();
        if (!tex->loadFromFile(resolved_path->string())) {
            std::cerr << "AssetManager: failed to load texture '" << std::string(path)
                      << "' from '" << resolved_path->string() << "'\n";
            return false;
        }

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
