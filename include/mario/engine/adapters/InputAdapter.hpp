#pragma once

#include "mario/engine/IInput.hpp"
#include "mario/input/InputManager.hpp"

#include <memory>

namespace mario::engine::adapters {
    class InputAdapter : public IInput {
    public:
        explicit InputAdapter(std::shared_ptr<mario::InputManager> i) : _input(std::move(i)) {}
        ~InputAdapter() override = default;

        [[nodiscard]] std::shared_ptr<mario::InputManager> underlying() const { return _input; }

    private:
        std::shared_ptr<mario::InputManager> _input;
    };
} // namespace mario::engine::adapters

