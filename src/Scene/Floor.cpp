#include "Floor.hpp"
#include <Objects/Attributes/Shapes/Plane.hpp>
#include <Objects/Attributes/Textures/CheckerTexture.hpp>

Floor::Floor(const Vector3D& _center, double elevation) :
Object(
    _center,
    std::make_shared<Plane>(Vector3D(0.0, 1.0, 0.0), -elevation),
    Material::Matte(std::make_shared<CheckerTexture>(Color{240, 240, 240}, Color{40, 40, 40}, 2.0, true))
) {}