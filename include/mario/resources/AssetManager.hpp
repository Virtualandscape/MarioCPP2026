#pragma once

#include <string_view>

namespace mario {

    // Textures, audio, fonts, caching
    class AssetManager {
    public:
        void load_texture(int id, std::string_view path);

        void load_sound(int id, std::string_view path);

        void unload_all();
    };
};
