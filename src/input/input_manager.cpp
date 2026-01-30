#include "mario/input/InputManager.hpp"

#include <SFML/Window/Keyboard.hpp>

namespace mario {

void InputManager::poll()
{
    set_action_state(Action::MoveLeft,
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A));
    set_action_state(Action::MoveRight,
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D));
    set_action_state(Action::Jump,
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up));
}

bool InputManager::is_pressed(Action action) const
{
    return _pressed[static_cast<std::size_t>(action)];
}

void InputManager::set_action_state(Action action, bool pressed)
{
    _pressed[static_cast<std::size_t>(action)] = pressed;
}

} // namespace mario
