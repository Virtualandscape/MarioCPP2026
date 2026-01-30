#include "mario/entities/Entity.hpp"

namespace mario {

void Entity::set_position(float x, float y)
{
    _x = x;
    _y = y;
}

void Entity::set_velocity(float vx, float vy)
{
    _vx = vx;
    _vy = vy;
}

void Entity::set_size(float width, float height)
{
    _width = width;
    _height = height;
}

float Entity::x() const { return _x; }
float Entity::y() const { return _y; }
float Entity::vx() const { return _vx; }
float Entity::vy() const { return _vy; }
float Entity::width() const { return _width; }
float Entity::height() const { return _height; }

void Entity::update_position(float dt)
{
    _x += _vx * dt;
    _y += _vy * dt;
}

} // namespace mario
