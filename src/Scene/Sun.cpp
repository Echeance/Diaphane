#include "Sun.hpp"

Sun::Sun(const Vector3D& _dir, const Color& _color, const double& _intensity) :
    enabled(true),
    direction(_dir.normalize()),
    color(_color),
    intensity(_intensity)
{}