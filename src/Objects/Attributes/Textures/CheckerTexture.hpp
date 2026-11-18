#ifndef CHECKERTEXTURE_HPP
#define CHECKERTEXTURE_HPP

#include <memory>
#include <Objects/Attributes/Textures/Texture.hpp>
#include <Objects/Attributes/Textures/SolidColor.hpp>

class CheckerTexture : public Texture {
private:
    std::shared_ptr<Texture> even;
    std::shared_ptr<Texture> odd;
    double scale;
    bool distance_fade;

public:
    CheckerTexture(const Color& _even = {240, 240, 240}, const Color& _odd = {40, 40, 40}, double _scale = 2.0, bool _distance_fade = true);
    CheckerTexture(const std::shared_ptr<Texture>& _even, const std::shared_ptr<Texture>& _odd, double _scale = 2.0, bool _distance_fade = false);

    Color value(const std::pair<double, double>& uv, const Vector3D& p) const override;
    double height(const std::pair<double, double>& uv, const Vector3D& p) const override;

    inline Color getEvenColor() const { return this->even->value({0, 0}, Vector3D()); }
    inline Color getOddColor() const { return this->odd->value({0, 0}, Vector3D()); }
    inline double getScale() const { return this->scale; }
    inline bool hasDistanceFade() const { return this->distance_fade; }
};

#endif // CHECKERTEXTURE_HPP