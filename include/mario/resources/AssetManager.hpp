#pragma once

#include <string_view>
#include <unordered_map>
#include <memory>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Font.hpp>
#include <mutex>
#include <queue>

namespace mario {

    // Textures, audio, fonts, caching
    class AssetManager {
    public:
        bool load_texture(int id, std::string_view path);

        std::shared_ptr<sf::Texture> get_mutable_texture(int id);
        std::shared_ptr<const sf::Texture> get_texture(int id) const;

        bool has_texture(int id) const;

        void load_sound(int id, std::string_view path);

        bool load_font(int id, std::string_view path);
        std::shared_ptr<const sf::Font> get_font(int id) const;
        bool has_font(int id) const;

        void unload_all();

        // Push an already-decoded image from background thread. Main thread must call finalize_decoded_images to create textures.
        void push_decoded_image(int id, sf::Image &&image);

        // Finalize any decoded images by creating sf::Texture objects on the main thread.
        void finalize_decoded_images();

    private:
        std::unordered_map<int, std::shared_ptr<sf::Texture>> _textures;
        std::unordered_map<int, std::shared_ptr<sf::Font>> _fonts;
        // Queue of decoded images waiting to be converted to textures on the main thread.
        std::mutex _pending_mutex;
        std::queue<std::pair<int, sf::Image>> _pending_images;
    };
};
