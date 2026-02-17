#pragma once

#include "mario/engine/IAssetManager.hpp"
#include "mario/engine/resources/AssetManager.hpp"

#include <memory>

namespace mario::engine::adapters {
    class AssetManagerAdapter : public IAssetManager {
    public:
        explicit AssetManagerAdapter(std::shared_ptr<mario::AssetManager> a) : _assets(std::move(a)) {}
        ~AssetManagerAdapter() override = default;

        bool load_texture(int id, const std::string &path) override { return _assets ? _assets->load_texture(id, path) : false; }
        std::shared_ptr<sf::Texture> get_mutable_texture(int id) override { return _assets ? _assets->get_mutable_texture(id) : nullptr; }
        std::shared_ptr<const sf::Texture> get_texture(int id) const override { return _assets ? _assets->get_texture(id) : nullptr; }
        bool has_texture(int id) const override { return _assets ? _assets->has_texture(id) : false; }

        bool load_font(int id, const std::string &path) override { return _assets ? _assets->load_font(id, path) : false; }
        std::shared_ptr<const sf::Font> get_font(int id) const override { return _assets ? _assets->get_font(id) : nullptr; }
        bool has_font(int id) const override { return _assets ? _assets->has_font(id) : false; }

        void unload_all() override { if (_assets) _assets->unload_all(); }

        void push_decoded_image(int id, sf::Image&& image) override { if (_assets) _assets->push_decoded_image(id, std::move(image)); }
        void finalize_decoded_images() override { if (_assets) _assets->finalize_decoded_images(); }

        [[nodiscard]] std::shared_ptr<mario::AssetManager> underlying() const { return _assets; }

    private:
        std::shared_ptr<mario::AssetManager> _assets;
    };
} // namespace mario::engine::adapters

