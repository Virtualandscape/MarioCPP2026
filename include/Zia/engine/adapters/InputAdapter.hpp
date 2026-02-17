#pragma once

#include "Zia/engine/IInput.hpp"
#include "Zia/engine/input/InputManager.hpp"

#include <memory>

namespace zia::engine::adapters {
    class InputAdapter : public IInput {
    public:
        explicit InputAdapter(std::shared_ptr<zia::InputManager> i) : _input(std::move(i)) {}
        ~InputAdapter() override = default;

        void poll() override { if (_input) _input->poll(); }
        bool is_pressed(zia::InputManager::Action action) const override { return _input ? _input->is_pressed(action) : false; }

        [[nodiscard]] std::shared_ptr<zia::InputManager> underlying() const { return _input; }

    private:
        std::shared_ptr<zia::InputManager> _input;
    };
} // namespace Zia::engine::adapters

