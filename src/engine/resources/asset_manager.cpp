// Implements the AssetManager class, which loads and manages textures and other assets for the game.
// Handles loading textures from disk, searching multiple locations, and reporting errors if assets are missing.

#include "mario/engine/resources/AssetManager.hpp"

#include <filesystem>
#include <iostream>
#include <optional>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Font.hpp>

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
        if (!resolved_path) return false;

        auto tex = std::make_shared<sf::Texture>();
        if (!tex->loadFromFile(resolved_path->string())) return false;
        tex->setSmooth(true);
        _textures[id] = tex;
        return true;
    }

    void AssetManager::push_decoded_image(int id, sf::Image &&image) {
        std::lock_guard<std::mutex> lock(_pending_mutex);
        _pending_images.emplace(id, std::move(image));
    }

    void AssetManager::finalize_decoded_images() {
        std::queue<std::pair<int, sf::Image>> local;
        {
            std::lock_guard<std::mutex> lock(_pending_mutex);
            std::swap(_pending_images, local);
        }

        while (!local.empty()) {
            auto &pr = local.front();
            int id = pr.first;
            sf::Image &img = pr.second;
            auto tex = std::make_shared<sf::Texture>();
            // create texture from image on main thread (OpenGL context owned here)
            bool tex_ok = tex->loadFromImage(img);
            if (!tex_ok) {
                std::cerr << "AssetManager: failed to create texture from decoded image for id=" << id << "\n";
            } else {
                tex->setSmooth(true);
                _textures[id] = tex;
            }
            local.pop();
        }
    }

    // Return a shared ownership pointer to the texture so callers don't hold raw pointers.
    std::shared_ptr<sf::Texture> AssetManager::get_mutable_texture(int id) {
        auto it = _textures.find(id);
        if (it == _textures.end()) return {};
        return it->second;
    }

    std::shared_ptr<const sf::Texture> AssetManager::get_texture(int id) const {
        auto it = _textures.find(id);
        if (it == _textures.end()) return {};
        return std::static_pointer_cast<const sf::Texture>(it->second);
    }

    bool AssetManager::has_texture(int id) const {
        return _textures.find(id) != _textures.end();
    }

    bool AssetManager::load_font(int id, std::string_view path) {
        if (path.empty()) return false;
        if (has_font(id)) return true;

        const auto resolved_path = resolve_asset_path(path);
        if (!resolved_path) return false;

        auto f = std::make_shared<sf::Font>();
        if (!f->openFromFile(resolved_path->string())) return false;
        _fonts[id] = f;
        return true;
    }

    std::shared_ptr<const sf::Font> AssetManager::get_font(int id) const {
        auto it = _fonts.find(id);
        if (it == _fonts.end()) return {};
        return std::static_pointer_cast<const sf::Font>(it->second);
    }

    bool AssetManager::has_font(int id) const {
        return _fonts.find(id) != _fonts.end();
    }

    void AssetManager::unload_all() {
        _textures.clear();
        _fonts.clear();
    }

} // namespace mario
