#ifndef PLANE_HPP
#define PLANE_HPP

#include "Shape.hpp"

class Plane : public Shape {
private:
    Vector3D normal; // Normale du plan (ex: (0, 1, 0) pour un sol horizontal)
    double d;        // Coefficient d'équation du plan (ax + by + cz + d = 0)

public:
    Plane(const Vector3D& normal, double d);

    double intersect(const Vector3D& center, const Ray& ray) const override;
    Vector3D normal_at(const Vector3D& center, const Vector3D& impact_point) const override;
    std::pair<double, double> get_uv(const Vector3D& center, const Vector3D& impact_point) const override;

    inline const Vector3D& getNormal() const { return this->normal; }
    inline double getD() const { return this->d; }
};

#endif