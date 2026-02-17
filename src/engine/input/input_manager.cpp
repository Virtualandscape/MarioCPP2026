#include "Zia/engine/input/InputManager.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Mouse.hpp>

#include <fstream>
#include <sstream>
#include <algorithm> // for std::remove_if
#include <cmath>

namespace zia {
    // Helper: map old enum actions to string names for backward compatibility.
    static std::string action_to_name(InputManager::Action a) {
        switch (a) {
            case InputManager::Action::MoveLeft: return "MoveLeft";
            case InputManager::Action::MoveRight: return "MoveRight";
            case InputManager::Action::Jump: return "Jump";
            case InputManager::Action::Escape: return "Escape";
            case InputManager::Action::ToggleDebug: return "ToggleDebug";
            default: return "Unknown";
        }
    }

    void InputManager::poll() {
        // Update legacy enum-based pressed states using keyboard hard-coded bindings.
        set_action_state(Action::MoveLeft,
                         sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(
                             sf::Keyboard::Key::A));
        set_action_state(Action::MoveRight,
                         sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(
                             sf::Keyboard::Key::D));
        set_action_state(Action::Jump,
                         sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || sf::Keyboard::isKeyPressed(
                             sf::Keyboard::Key::Up));
        set_action_state(Action::Escape,
                         sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape));

        // Map the H key to ToggleDebug action to show/hide bounding boxes
        set_action_state(Action::ToggleDebug,
                         sf::Keyboard::isKeyPressed(sf::Keyboard::Key::H));

        // Update named actions based on bindings.
        for (auto &pair : _bindings) {
            const std::string &name = pair.first;
            bool anyActive = false;
            for (auto const &b : pair.second) {
                switch (b.device) {
                    case InputDevice::Keyboard:
                        if (b.code >= 0) {
                            // SFML Key codes are int convertible from sf::Keyboard::Key
                            if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(b.code))) {
                                anyActive = true;
                            }
                        }
                        break;
                    case InputDevice::MouseButton:
                        if (sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(b.code))) {
                            anyActive = true;
                        }
                        break;
                    case InputDevice::GamepadButton:
                        // SFML uses joystick button checks; joystickId -1 means any joystick
                        if (b.joystickId < 0) {
                            // check all connected joysticks
                            for (unsigned i = 0; i < sf::Joystick::Count; ++i) {
                                if (sf::Joystick::isConnected(i) && sf::Joystick::isButtonPressed(i, b.code)) {
                                    anyActive = true;
                                    break;
                                }
                            }
                        } else {
                            if (b.joystickId < static_cast<int>(sf::Joystick::Count) && sf::Joystick::isConnected(b.joystickId) && sf::Joystick::isButtonPressed(b.joystickId, b.code)) {
                                anyActive = true;
                            }
                        }
                        break;
                    case InputDevice::GamepadAxis: {
                        float pos = 0.f;
                        if (b.joystickId < 0) {
                            for (unsigned i = 0; i < sf::Joystick::Count; ++i) {
                                if (!sf::Joystick::isConnected(i)) continue;
                                pos = sf::Joystick::getAxisPosition(i, static_cast<sf::Joystick::Axis>(b.code));
                                // SFML axis range is -100 .. 100
                                float normalized = (b.axisPositive ? pos : -pos) / 100.f;
                                if (normalized >= b.axisThreshold) {
                                    anyActive = true;
                                    break;
                                }
                            }
                        } else {
                            if (b.joystickId >= 0 && b.joystickId < static_cast<int>(sf::Joystick::Count) && sf::Joystick::isConnected(b.joystickId)) {
                                pos = sf::Joystick::getAxisPosition(b.joystickId, static_cast<sf::Joystick::Axis>(b.code));
                                float normalized = (b.axisPositive ? pos : -pos) / 100.f;
                                if (normalized >= b.axisThreshold) {
                                    anyActive = true;
                                }
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
                if (anyActive) break;
            }
            // Update edge state maps
            bool prev = _pressed_by_name[name];
            _pressed_by_name[name] = anyActive;
            _down_by_name[name] = (!prev && anyActive);
            _released_by_name[name] = (prev && !anyActive);
        }

        // If capturing, poll for a pressed key/button to capture.
        if (_capturing) {
            // Poll keyboard: SFML does not expose a portable KeyCount constant in all versions; use a reasonable upper bound.
            const int maxKey = 256; // safe bound for key codes
            for (int k = 0; k < maxKey; ++k) {
                if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(k))) {
                    Binding b;
                    b.device = InputDevice::Keyboard;
                    b.code = k;
                    _captured_pending = b;
                    _capturing = false; // stop capturing after first event
                    break;
                }
            }
            // Poll mouse buttons: assume up to 8 buttons
            if (!_captured_pending.has_value()) {
                const int maxMouse = 8;
                for (int m = 0; m < maxMouse; ++m) {
                    if (sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(m))) {
                        Binding b;
                        b.device = InputDevice::MouseButton;
                        b.code = m;
                        _captured_pending = b;
                        _capturing = false;
                        break;
                    }
                }
            }
            // Poll joystick buttons
            if (!_captured_pending.has_value()) {
                for (unsigned jid = 0; jid < sf::Joystick::Count; ++jid) {
                    if (!sf::Joystick::isConnected(jid)) continue;
                    unsigned btnCount = sf::Joystick::getButtonCount(jid);
                    for (unsigned bi = 0; bi < btnCount; ++bi) {
                        if (sf::Joystick::isButtonPressed(jid, bi)) {
                            Binding b;
                            b.device = InputDevice::GamepadButton;
                            b.code = static_cast<int>(bi);
                            b.joystickId = static_cast<int>(jid);
                            _captured_pending = b;
                            _capturing = false;
                            break;
                        }
                    }
                    if (_captured_pending.has_value()) break;
                }
            }
            // Poll joystick axes roughly: assume up to 8 axes
            if (!_captured_pending.has_value()) {
                const int maxAxes = 8;
                for (unsigned jid = 0; jid < sf::Joystick::Count; ++jid) {
                    if (!sf::Joystick::isConnected(jid)) continue;
                    for (int axis = 0; axis < maxAxes; ++axis) {
                        float pos = sf::Joystick::getAxisPosition(jid, static_cast<sf::Joystick::Axis>(axis));
                        if (std::abs(pos) > 50.f) { // only capture if moved significantly
                            Binding b;
                            b.device = InputDevice::GamepadAxis;
                            b.code = axis;
                            b.joystickId = static_cast<int>(jid);
                            b.axisPositive = pos > 0.f;
                            b.axisThreshold = 0.5f;
                            _captured_pending = b;
                            _capturing = false;
                            break;
                        }
                    }
                    if (_captured_pending.has_value()) break;
                }
            }
        }
    }

    bool InputManager::is_pressed(Action action) const {
        return _pressed[static_cast<std::size_t>(action)];
    }

    bool InputManager::is_pressed(const std::string &action) const {
        auto it = _pressed_by_name.find(action);
        return it != _pressed_by_name.end() ? it->second : false;
    }

    bool InputManager::is_down(const std::string &action) const {
        auto it = _down_by_name.find(action);
        return it != _down_by_name.end() ? it->second : false;
    }

    bool InputManager::is_released(const std::string &action) const {
        auto it = _released_by_name.find(action);
        return it != _released_by_name.end() ? it->second : false;
    }

    std::vector<Binding> InputManager::get_bindings(const std::string &action) const {
        auto it = _bindings.find(action);
        if (it == _bindings.end()) return {};
        return it->second;
    }

    void InputManager::set_bindings(const std::string &action, const std::vector<Binding> &bindings) {
        _bindings[action] = bindings;
    }

    void InputManager::add_binding(const std::string &action, Binding const &binding) {
        auto &vec = _bindings[action];
        // avoid duplicates
        for (auto const &b : vec) if (b == binding) return;
        vec.push_back(binding);
    }

    void InputManager::remove_binding(const std::string &action, Binding const &binding) {
        auto it = _bindings.find(action);
        if (it == _bindings.end()) return;
        auto &vec = it->second;
        vec.erase(std::remove_if(vec.begin(), vec.end(), [&](Binding const &b) { return b == binding; }), vec.end());
    }

    // Simple persistence format: one binding per line: action device code joystick axisPositive axisThreshold
    // TODO: replace with JSON (nlohmann::json) for robustness.
    void InputManager::load_bindings_from_file(const std::string &path) {
        std::ifstream in(path);
        if (!in) return; // no file: silently ignore
        _bindings.clear();
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string action;
            int device;
            int code;
            int jid;
            int axisPosInt;
            float thresh;
            if (!(ss >> action >> device >> code >> jid >> axisPosInt >> thresh)) continue;
            Binding b;
            b.device = static_cast<InputDevice>(device);
            b.code = code;
            b.joystickId = jid;
            b.axisPositive = (axisPosInt != 0);
            b.axisThreshold = thresh;
            _bindings[action].push_back(b);
        }
    }

    void InputManager::save_bindings_to_file(const std::string &path) const {
        std::ofstream out(path);
        if (!out) return;
        for (auto const &pair : _bindings) {
            const auto &action = pair.first;
            for (auto const &b : pair.second) {
                out << action << ' ' << static_cast<int>(b.device) << ' ' << b.code << ' '
                    << b.joystickId << ' ' << (b.axisPositive ? 1 : 0) << ' ' << b.axisThreshold << '\n';
            }
        }
    }

    void InputManager::start_capture(const std::string &action) {
        _capturing = true;
        _capture_action = action;
        _captured_pending.reset();
    }

    void InputManager::stop_capture() {
        _capturing = false;
        _captured_pending.reset();
    }

    bool InputManager::is_capturing() const {
        return _capturing;
    }

    std::optional<Binding> InputManager::poll_captured_binding() {
        if (_captured_pending.has_value()) {
            Binding b = *_captured_pending;
            _captured_pending.reset();
            return b;
        }
        return std::nullopt;
    }

    void InputManager::set_action_state(Action action, bool pressed) {
        _pressed[static_cast<std::size_t>(action)] = pressed;
        // also mirror into named action map if exists
        const std::string name = action_to_name(action);
        if (!name.empty() && name != "Unknown") {
            bool prev = _pressed_by_name[name];
            _pressed_by_name[name] = pressed;
            _down_by_name[name] = (!prev && pressed);
            _released_by_name[name] = (prev && !pressed);
        }
    }

    // Implement Binding::to_string (debug helper)
    std::string Binding::to_string() const {
        std::ostringstream ss;
        ss << "dev=" << static_cast<int>(device) << " code=" << code << " jid=" << joystickId << " pos=" << axisPositive << " thr=" << axisThreshold;
        return ss.str();
    }
} // namespace Zia
