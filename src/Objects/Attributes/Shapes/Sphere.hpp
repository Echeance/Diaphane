#ifndef SPHERE_HPP
#define SPHERE_HPP

#include <Maths/Vector.hpp>
#include <Lightning/Ray.hpp>
#include <Objects/Attributes/Shapes/Shape.hpp>

class Sphere : public Shape {
private:
    double radius;

public:
    Sphere(double _radius = .0);

    inline const double& getRadius() const { return this->radius; };
    inline double& getRadius() { return this->radius; };

    double intersect(const Vector3D& center, const Ray& ray) const override;
    Vector3D normal_at(const Vector3D& center, const Vector3D& impact_point) const override;
    std::pair<double, double> get_uv(const Vector3D& center, const Vector3D& impact_point) const override;
};

#endif // SPHERE_HPP