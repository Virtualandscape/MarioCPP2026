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

        [[nodiscard]] bool is_pressed(zia::InputManager::Action action) const override { return _input ? _input->is_pressed(action) : false; }

        [[nodiscard]] bool is_pressed(const std::string &action) const override { return _input ? _input->is_pressed(action) : false; }
        [[nodiscard]] bool is_down(const std::string &action) const override { return _input ? _input->is_down(action) : false; }
        [[nodiscard]] bool is_released(const std::string &action) const override { return _input ? _input->is_released(action) : false; }

        [[nodiscard]] std::vector<zia::Binding> get_bindings(const std::string &action) const override { return _input ? _input->get_bindings(action) : std::vector<zia::Binding>{}; }
        void set_bindings(const std::string &action, const std::vector<zia::Binding> &bindings) override { if (_input) _input->set_bindings(action, bindings); }
        void add_binding(const std::string &action, zia::Binding const &binding) override { if (_input) _input->add_binding(action, binding); }
        void remove_binding(const std::string &action, zia::Binding const &binding) override { if (_input) _input->remove_binding(action, binding); }

        void load_bindings_from_file(const std::string &path) override { if (_input) _input->load_bindings_from_file(path); }
        void save_bindings_to_file(const std::string &path) const override { if (_input) _input->save_bindings_to_file(path); }

        void start_capture(const std::string &action) override { if (_input) _input->start_capture(action); }
        void stop_capture() override { if (_input) _input->stop_capture(); }
        [[nodiscard]] bool is_capturing() const override { return _input ? _input->is_capturing() : false; }
        std::optional<zia::Binding> poll_captured_binding() override { return _input ? _input->poll_captured_binding() : std::optional<zia::Binding>{}; }

        [[nodiscard]] std::shared_ptr<zia::InputManager> underlying() const { return _input; }

    private:
        std::shared_ptr<zia::InputManager> _input;
    };
} // namespace Zia::engine::adapters

