// Implements the AssetManager class, which loads and manages textures and other assets for the game.
// Handles loading textures from disk, searching multiple locations, and reporting errors if assets are missing.

#include "mario/resources/AssetManager.hpp"

#include <filesystem>
#include <iostream>
#include <optional>
#include <chrono>
#include <fstream>

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

        // Time the asset path resolution and file load separately to detect which step is slow.
        const auto t0 = std::chrono::steady_clock::now();
        const auto resolved_path = resolve_asset_path(path);
        const auto t1 = std::chrono::steady_clock::now();

        if (!resolved_path) {
            std::cerr << "AssetManager: failed to find texture file '" << std::string(path) << "'\n";
            return false;
        }

        // Allocate texture and time the SFML loadFromFile call which may be expensive (IO + decode).
        auto tex = std::make_shared<sf::Texture>();
        const auto t2 = std::chrono::steady_clock::now();
        if (!tex->loadFromFile(resolved_path->string())) {
            std::cerr << "AssetManager: failed to load texture '" << std::string(path)
                      << "' from '" << resolved_path->string() << "'\n";
            return false;
        }
        const auto t3 = std::chrono::steady_clock::now();

        // Log timings: resolution, allocation, load (in milliseconds). This helps find which load causes hitches.
        const auto resolve_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        const auto alloc_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        const auto load_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

        std::cerr << "AssetManager::load_texture timings for '" << std::string(path) << "' -> resolve="
                  << resolve_ms << "ms alloc=" << alloc_ms << "ms load=" << load_ms << "ms\n";

        // Also append timings to a startup log file to capture logs when running as a GUI app.
        try {
            std::ofstream ofs("startup_asset_log.txt", std::ios::app);
            if (ofs) {
                ofs << "AssetManager::load_texture timings for '" << std::string(path) << "' -> resolve="
                    << resolve_ms << "ms alloc=" << alloc_ms << "ms load=" << load_ms << "ms\n";
            }
        } catch (...) {
            // Ignore filesystem/logging errors to avoid impacting game behavior.
        }

        tex->setSmooth(true);
        _textures[id] = tex;
        return true;
    }

    // Return a shared ownership pointer to the texture so callers don't hold raw pointers.
    // This matches the declaration in the header and avoids returning raw pointers.
    std::shared_ptr<sf::Texture> AssetManager::get_mutable_texture(int id) {
        auto it = _textures.find(id);
        if (it == _textures.end()) return std::shared_ptr<sf::Texture>();
        return it->second;
    }

    // Return a shared_ptr to a const texture. We cast the stored shared_ptr<sf::Texture>
    // to shared_ptr<const sf::Texture> for callers that need a read-only view.
    std::shared_ptr<const sf::Texture> AssetManager::get_texture(int id) const {
        auto it = _textures.find(id);
        if (it == _textures.end()) return std::shared_ptr<const sf::Texture>();
        return std::static_pointer_cast<const sf::Texture>(it->second);
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
