#include "Ray.hpp"

Ray::Ray(Vector3D _origin, Vector3D _dir) : origin(_origin), dir(_dir) {};

Vector3D Ray::point_at(double dist) const {
    return this->origin + this->dir * dist;
}