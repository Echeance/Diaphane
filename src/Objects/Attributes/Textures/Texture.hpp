#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <utility>

#include <Maths/Vector.hpp>
#include <Color/Color.hpp>

class Texture {
public:
    Texture() = default;
    virtual ~Texture() = default;

    virtual Color value(const std::pair<double, double>& uv, const Vector3D& p) const = 0;

    // Hauteur scalaire utilisée pour perturber la normale (Bump Mapping)
    virtual double height(const std::pair<double, double>&, const Vector3D&) const {
        return 0.0;
    }
};

#endif // TEXTURE_HPP