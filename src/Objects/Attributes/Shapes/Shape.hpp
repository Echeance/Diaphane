#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <utility>

#include <Maths/Vector.hpp>
#include <Lightning/Ray.hpp>

class Shape {
public:
    Shape() = default;
    virtual ~Shape() = default;

    virtual double intersect(const Vector3D& center, const Ray& ray) const = 0;
    virtual Vector3D normal_at(const Vector3D& center, const Vector3D& impact_point) const = 0;
    virtual std::pair<double, double> get_uv(const Vector3D& center, const Vector3D& impact_point) const = 0;
};

#endif // SHAPE_HPP