#include "Vector.hpp"
#include <cmath>

Vector3D::Vector3D(const double& _x, const double& _y, const double& _z) : x(_x), y(_y), z(_z) {};

Vector3D Vector3D::operator+(const Vector3D& vec) const {
    return Vector3D(this->x + vec.x, this->y + vec.y, this->z + vec.z);
}

Vector3D Vector3D::operator-(const Vector3D& vec) const {
    return Vector3D(this->x - vec.x, this->y - vec.y, this->z - vec.z);
}

Vector3D Vector3D::operator*(const Vector3D& vec) const {
    return Vector3D(this->x * vec.x, this->y * vec.y, this->z * vec.z);
}

Vector3D Vector3D::operator*(const double& scal) const {
    return Vector3D(this->x * scal, this->y * scal, this->z * scal);
}

Vector3D operator*(const double& scal, const Vector3D& vec) {
    return vec * scal;
}

Vector3D Vector3D::operator/(const double& scal) const {
    return Vector3D(this->x / scal, this->y / scal, this->z / scal);
}

double Vector3D::dot_product(const Vector3D& vec) const {
    return this->x * vec.x + this->y * vec.y + this->z * vec.z;
}

Vector3D Vector3D::vector_product(const Vector3D& vec) const {
    return Vector3D(
        this->y * vec.z - this->z * vec.y,
        this->z * vec.x - this->x * vec.z,
        this->x * vec.y - this->y * vec.x
    );
}

double Vector3D::length() const {
    return std::sqrt(this->square_length());
}

Vector3D Vector3D::normalize() const {
    double len = this->length();
    if (len == 0)
        return Vector3D();

    return (*this) * (1.0 / len);
}

Vector3D Vector3D::operator-() const {
    return Vector3D(-this->x, -this->y, -this->z);
}