#include "Ball.hpp"

#include <Objects/Attributes/Shapes/Sphere.hpp>

Ball::Ball(const Vector3D& center, double radius, const std::shared_ptr<Material>& _material) : Object(center, std::make_shared<Sphere>(radius), _material) {}