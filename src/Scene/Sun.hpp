#ifndef SUN_HPP
#define SUN_HPP

#include <Maths/Vector.hpp>
#include <Color/Color.hpp>

struct Sun {
    bool enabled;
    Vector3D direction;
    Color color;
    double intensity;

    Sun(const Vector3D& _dir = Vector3D(0.0, 1.0, 0.0), const Color& _color = Color{255, 250, 235}, const double& _intensity = 1.0);
};

#endif // SUN_HPP