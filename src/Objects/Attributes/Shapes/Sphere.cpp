#include "Sphere.hpp"

#include <cmath>
#include <algorithm>

#include <Maths/Random.hpp>

Sphere::Sphere(double _radius) : Shape(), radius(_radius) {};

double Sphere::intersect(const Vector3D& center, const Ray& ray) const {
    Vector3D oc = ray.getOrigin() - center;

    double a = ray.getDirection().dot_product(ray.getDirection());
    double b = 2 * (ray.getDirection().dot_product(oc));
    double c = oc.dot_product(oc) - this->radius * this->radius;

    double discriminant = (b * b) - (4.0 * a * c);

    if (discriminant < 0.0) {
        return -1.0; // Le rayon passe à côté de la sphère
    }

    double sqr_disc = std::sqrt(discriminant);
    double t1 = (-b - sqr_disc) / (2.0 * a);
    double t2 = (-b + sqr_disc) / (2.0 * a);

    // On retourne la première intersection valide strictement supérieure à l'epsilon
    if (t1 > 0.001) return t1;
    if (t2 > 0.001) return t2;

    return -1.0; // Aucune intersection valide devant le rayon
}

Vector3D Sphere::normal_at(const Vector3D& center, const Vector3D& impact_point) const {
    return (impact_point - center) / this->radius;
}

std::pair<double, double> Sphere::get_uv(const Vector3D& center, const Vector3D& impact_point) const {
    Vector3D p = this->normal_at(center, impact_point);
    double theta = std::atan2(p.getZ(), p.getX());
    double phi = std::asin(std::clamp(p.getY(), -1.0, 1.0));

    double u = 0.5 + theta / (2.0 * M_PI);
    double v = 0.5 + phi / M_PI;
    return {u, v};
}