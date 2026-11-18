#include "Object.hpp"

Object::Object(const Vector3D& _center, std::shared_ptr<Shape> _shape, const std::shared_ptr<Material>& _material) : center(_center), shape(std::move(_shape)), material(_material) {}