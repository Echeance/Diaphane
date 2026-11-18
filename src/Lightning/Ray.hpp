#ifndef RAY_HPP
#define RAY_HPP

#include <Maths/Vector.hpp>

class Ray {
private:
    Vector3D origin, dir;

public:
    Ray(Vector3D _origin = Vector3D(), Vector3D _dir = Vector3D());

    inline const Vector3D& getOrigin() const { return this->origin; };
    inline Vector3D& getOrigin() { return this->origin; };

    inline const Vector3D& getDirection() const { return this->dir; };
    inline Vector3D& getDirection() { return this->dir; };

    Vector3D point_at(double dist) const;
};

#endif // RAY_HPP