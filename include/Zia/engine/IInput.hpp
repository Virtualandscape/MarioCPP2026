#pragma once

#include <string>
#include <vector>
#include <optional>

#include "Zia/engine/input/Binding.hpp"
#include "Zia/engine/input/InputManager.hpp"

namespace zia::engine {
    class IInput {
    public:
        virtual ~IInput() = default;

        // Poll pending input events (typically called once per frame).
        virtual void poll() = 0;

        // Legacy: query whether an enum action is currently pressed.
        // Kept for compatibility with existing code that uses zia::InputManager::Action.
        [[nodiscard]] virtual bool is_pressed(zia::InputManager::Action action) const = 0;

        // New string-based action API.
        [[nodiscard]] virtual bool is_pressed(const std::string &action) const = 0;
        [[nodiscard]] virtual bool is_down(const std::string &action) const = 0;
        [[nodiscard]] virtual bool is_released(const std::string &action) const = 0;

        // Bindings management
        [[nodiscard]] virtual std::vector<zia::Binding> get_bindings(const std::string &action) const = 0;
        virtual void set_bindings(const std::string &action, const std::vector<zia::Binding> &bindings) = 0;
        virtual void add_binding(const std::string &action, zia::Binding const &binding) = 0;
        virtual void remove_binding(const std::string &action, zia::Binding const &binding) = 0;

        // Persistence
        virtual void load_bindings_from_file(const std::string &path) = 0;
        virtual void save_bindings_to_file(const std::string &path) const = 0;

        // Capture mode for UI-driven remapping
        virtual void start_capture(const std::string &action) = 0;
        virtual void stop_capture() = 0;
        [[nodiscard]] virtual bool is_capturing() const = 0;
        virtual std::optional<zia::Binding> poll_captured_binding() = 0;
    };
} // namespace zia::engine
