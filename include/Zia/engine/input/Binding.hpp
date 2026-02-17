#pragma once

#include <cstdint>
#include <string>

namespace zia {
    // Input device types supported by bindings.
    enum class InputDevice : std::uint8_t {
        Keyboard = 0,
        MouseButton = 1,
        GamepadButton = 2,
        GamepadAxis = 3
    };

    // Represents a single input binding (device + id + optional joystick index/axis info).
    struct Binding {
        // Which device this binding comes from (keyboard, mouse, gamepad button/axis).
        InputDevice device = InputDevice::Keyboard;
        // Device-specific code (keyboard scancode / mouse button / gamepad button or axis id).
        int code = -1;
        // Joystick id for gamepad bindings. 0 = first joystick. -1 means any joystick.
        int joystickId = 0;
        // For axis bindings, whether the positive direction triggers the action.
        bool axisPositive = true;
        // Threshold for axis activation (0..1). Ignored for buttons/keys.
        float axisThreshold = 0.5f;

        // Comparison operator to allow removal/search of bindings.
        bool operator==(Binding const& other) const noexcept {
            return device == other.device && code == other.code && joystickId == other.joystickId
                   && axisPositive == other.axisPositive && std::abs(axisThreshold - other.axisThreshold) < 1e-6f;
        }

        // Human readable debug representation.
        [[nodiscard]] std::string to_string() const;
    };
} // namespace zia

