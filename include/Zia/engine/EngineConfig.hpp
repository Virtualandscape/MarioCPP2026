#pragma once

#include <functional>
#include <cstddef>
#include <map>

namespace zia::engine {
    // Simple engine configuration holder with observer callbacks.
    class EngineConfig {
    public:
        using Observer = std::function<void(const EngineConfig&)>;
        using ObserverId = std::size_t;

        EngineConfig(int width = 800, int height = 480, bool fullscreen = false, float masterVolume = 1.0f);

        // Getters
        int window_width() const;
        int window_height() const;
        bool fullscreen() const;
        float master_volume() const;

        // Setters (notify observers on change)
        void set_window_size(int width, int height);
        void set_fullscreen(bool enabled);
        void set_master_volume(float volume);

        // Observer management
        ObserverId register_observer(Observer cb);
        void unregister_observer(ObserverId id);

    private:
        int _width;
        int _height;
        bool _fullscreen;
        float _master_volume;

        std::map<ObserverId, Observer> _observers;
        ObserverId _next_id = 1;

        void notify_all() const;
    };
} // namespace zia::engine

