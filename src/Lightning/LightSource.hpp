#ifndef LIGHTSOURCE_HPP
#define LIGHTSOURCE_HPP

#include <Maths/Vector.hpp>

struct LightSource {
    Vector3D pos;
    double intensity;
    double radius;

    LightSource(Vector3D _pos = Vector3D(), double _intensity = 1.0, double _radius = 0.0);
};

#endif // LIGHTSOURCE_HPP