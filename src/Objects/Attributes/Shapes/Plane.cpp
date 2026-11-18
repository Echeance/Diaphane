#include "Plane.hpp"

#include <cmath>

Plane::Plane(const Vector3D& _normal, double _d) : Shape(), normal(_normal.normalize()), d(_d) {}

double Plane::intersect(const Vector3D&, const Ray& ray) const {
    // Produit scalaire entre la direction du rayon et la normale du plan
    double denom = this->normal.dot_product(ray.getDirection());

    // Si le dénominateur est proche de 0, le rayon est parallèle au plan
    if (std::fabs(denom) < 1e-6) {
        return -1.0;
    }

    // t = -(dot(ray.origin, normal) + d) / dot(ray.direction, normal)
    double t = -(this->normal.dot_product(ray.getOrigin()) + this->d) / denom;

    if (t > 0.001) {
        return t;
    }

    return -1.0;
}

Vector3D Plane::normal_at(const Vector3D&, const Vector3D&) const {
    // La normale d'un plan est constante partout
    return this->normal;
}

std::pair<double, double> Plane::get_uv(const Vector3D&, const Vector3D& impact_point) const {
    return {impact_point.getX(), impact_point.getZ()};
}