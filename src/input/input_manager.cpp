#include "mario/input/InputManager.hpp"

namespace mario {

void InputManager::poll() {}
bool InputManager::is_pressed(int action) const
{
    (void)action;
    return false;
}

} // namespace mario
