#pragma once

#include "mario/engine/IAssetManager.hpp"
#include "mario/resources/AssetManager.hpp"

#include <memory>

namespace mario::engine::adapters {
    class AssetManagerAdapter : public IAssetManager {
    public:
        explicit AssetManagerAdapter(std::shared_ptr<mario::AssetManager> a) : _assets(std::move(a)) {}
        ~AssetManagerAdapter() override = default;

        [[nodiscard]] std::shared_ptr<mario::AssetManager> underlying() const { return _assets; }

    private:
        std::shared_ptr<mario::AssetManager> _assets;
    };
} // namespace mario::engine::adapters

