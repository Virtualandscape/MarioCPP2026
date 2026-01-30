#include "mario/entities/Entity.hpp"

namespace mario {

void Entity::set_position(float x, float y)
{
    x_ = x;
    y_ = y;
}

void Entity::set_velocity(float vx, float vy)
{
    vx_ = vx;
    vy_ = vy;
}

void Entity::set_size(float width, float height)
{
    width_ = width;
    height_ = height;
}

float Entity::x() const { return x_; }
float Entity::y() const { return y_; }
float Entity::vx() const { return vx_; }
float Entity::vy() const { return vy_; }
float Entity::width() const { return width_; }
float Entity::height() const { return height_; }

void Entity::integrate(float dt)
{
    x_ += vx_ * dt;
    y_ += vy_ * dt;
}

} // namespace mario
