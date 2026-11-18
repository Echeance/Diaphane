#ifndef VECTOR_HPP
#define VECTOR_HPP

class Vector3D {
private:
    double x, y, z;

public:
    Vector3D(const double& _x = .0, const double& _y = .0, const double& _z = .0);

    Vector3D operator+(const Vector3D& vec) const;
    Vector3D operator-(const Vector3D& vec) const;
    Vector3D operator*(const Vector3D& vec) const;
    friend Vector3D operator*(const double& scal, const Vector3D &vec);
    Vector3D operator*(const double& scal) const;
    Vector3D operator/(const double& scal) const;
    Vector3D operator-() const;

    inline const double& getX() const { return this->x; };
    inline double& getX() { return this->x; };

    inline const double& getY() const { return this->y; };
    inline double& getY() { return this->y; };

    inline const double& getZ() const { return this->z; };
    inline double& getZ() { return this->z; };

    double dot_product(const Vector3D &vec) const;
    Vector3D vector_product(const Vector3D &vec) const;

    double length() const;
    inline double square_length() const { return x * x + y * y + z * z; }
    Vector3D normalize() const;
};

#endif // VECTOR_HPP