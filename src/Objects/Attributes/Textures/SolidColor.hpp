#ifndef SOLIDCOLOR_HPP
#define SOLIDCOLOR_HPP

#include <Objects/Attributes/Textures/Texture.hpp>

class SolidColor : public Texture {
private:
    Color color;

public:
    SolidColor(const Color& _color);

    Color value(const std::pair<double, double>& uv, const Vector3D& p) const override;
};

#endif // SOLIDCOLOR_HPP