#include "Zia/engine/EngineConfig.hpp"
#include <algorithm>

namespace zia::engine {
    EngineConfig::EngineConfig(int width, int height, bool fullscreen, float masterVolume)
        : _width(width), _height(height), _fullscreen(fullscreen), _master_volume(masterVolume) {}

    int EngineConfig::window_width() const { return _width; }
    int EngineConfig::window_height() const { return _height; }
    bool EngineConfig::fullscreen() const { return _fullscreen; }
    float EngineConfig::master_volume() const { return _master_volume; }

    void EngineConfig::set_window_size(int width, int height) {
        _width = std::max(1, width);
        _height = std::max(1, height);
        notify_all();
    }

    void EngineConfig::set_fullscreen(bool enabled) {
        _fullscreen = enabled;
        notify_all();
    }

    void EngineConfig::set_master_volume(float volume) {
        _master_volume = std::clamp(volume, 0.0f, 1.0f);
        notify_all();
    }

    EngineConfig::ObserverId EngineConfig::register_observer(Observer cb) {
        if (!cb) return 0;
        const auto id = _next_id++;
        _observers.emplace(id, std::move(cb));
        return id;
    }

    void EngineConfig::unregister_observer(ObserverId id) {
        _observers.erase(id);
    }

    void EngineConfig::notify_all() const {
        for (const auto &p : _observers) {
            if (p.second) p.second(*this);
        }
    }
} // namespace zia::engine

