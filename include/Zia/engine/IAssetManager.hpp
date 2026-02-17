#pragma once

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <string>
#include <memory>

namespace zia::engine {
    class IAssetManager {
    public:
        virtual ~IAssetManager() = default;

        virtual bool load_texture(int id, const std::string &path) = 0;
        virtual std::shared_ptr<sf::Texture> get_mutable_texture(int id) = 0;
        virtual std::shared_ptr<const sf::Texture> get_texture(int id) const = 0;
        virtual bool has_texture(int id) const = 0;

        virtual bool load_font(int id, const std::string &path) = 0;
        virtual std::shared_ptr<const sf::Font> get_font(int id) const = 0;
        virtual bool has_font(int id) const = 0;

        virtual void unload_all() = 0;

        // Multi-threaded decode/finalize helpers used by PlayScene.
        virtual void push_decoded_image(int id, sf::Image&& image) = 0;
        virtual void finalize_decoded_images() = 0;
    };
} // namespace Zia::engine

