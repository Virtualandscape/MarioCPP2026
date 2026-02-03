#pragma once

#include <string_view>
#include <unordered_map>
#include <memory>

#include <SFML/Graphics/Texture.hpp>

namespace mario {

    // Textures, audio, fonts, caching
    class AssetManager {
    public:
        bool load_texture(int id, std::string_view path);

        const sf::Texture* get_texture(int id) const;

        bool has_texture(int id) const;

        void load_sound(int id, std::string_view path);

        void unload_all();

    private:
        std::unordered_map<int, std::shared_ptr<sf::Texture>> _textures;
    };
};
