#include "SolidColor.hpp"

SolidColor::SolidColor(const Color& _color) : color(_color) {}

Color SolidColor::value(const std::pair<double, double>&, const Vector3D&) const {
    return this->color;
}