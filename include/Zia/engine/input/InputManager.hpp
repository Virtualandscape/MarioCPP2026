#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <algorithm>
#include "Zia/engine/input/Binding.hpp"

namespace zia {
    // Key bindings, edge detection (pressed/released).
    class InputManager {
    public:
        enum class Action {
            MoveLeft,
            MoveRight,
            Jump,
            Escape,
            ToggleDebug, // toggle debug overlay (bounding boxes)
            Count
        };

        // Poll low-level inputs and update action states.
        void poll();

        // Old API: query by enum action (kept for backward compatibility).
        [[nodiscard]] bool is_pressed(Action action) const;

        // New string-based API for actions.
        [[nodiscard]] bool is_pressed(const std::string &action) const;
        [[nodiscard]] bool is_down(const std::string &action) const;
        [[nodiscard]] bool is_released(const std::string &action) const;

        // Bindings management
        [[nodiscard]] std::vector<Binding> get_bindings(const std::string &action) const;
        void set_bindings(const std::string &action, const std::vector<Binding> &bindings);
        void add_binding(const std::string &action, Binding const &binding);
        void remove_binding(const std::string &action, Binding const &binding);

        // Persistence
        void load_bindings_from_file(const std::string &path);
        void save_bindings_to_file(const std::string &path) const;

        // Capture mode for remapping via UI: start capturing events for the given action.
        void start_capture(const std::string &action);
        void stop_capture();
        [[nodiscard]] bool is_capturing() const;
        // Poll once to obtain a captured binding (if any) while in capture mode.
        std::optional<Binding> poll_captured_binding();

        // Set action state directly (used internally and for compatibility).
        void set_action_state(Action action, bool pressed);

    private:
        std::array<bool, static_cast<std::size_t>(Action::Count)> _pressed = {};

        // New: mapping from action name to list of Bindings
        std::unordered_map<std::string, std::vector<Binding>> _bindings;

        // Edge detection / state by action name
        std::unordered_map<std::string, bool> _pressed_by_name;
        std::unordered_map<std::string, bool> _down_by_name;
        std::unordered_map<std::string, bool> _released_by_name;

        // Capture state
        bool _capturing = false;
        std::string _capture_action;
        std::optional<Binding> _captured_pending;
    };
} // namespace Zia
