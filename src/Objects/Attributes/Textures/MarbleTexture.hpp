#ifndef MARBLETEXTURE_HPP
#define MARBLETEXTURE_HPP

#include <array>
#include <Objects/Attributes/Textures/Texture.hpp>

class MarbleTexture : public Texture {
private:
    Color color1;
    Color color2;
    double scale;
    double turbulence_strength;
    std::array<int, 512> perm;

    double fade(double t) const;
    double lerp(double t, double a, double b) const;
    double grad(int hash, double x, double y, double z) const;
    double noise(const Vector3D& p) const;
    double turbulence(const Vector3D& p, int depth = 5) const;

public:
    MarbleTexture(const Color& _color1 = {245, 245, 240}, const Color& _color2 = {45, 55, 65}, double _scale = 3.0, double _turbulence_strength = 6.0);

    Color value(const std::pair<double, double>& uv, const Vector3D& p) const override;
    double height(const std::pair<double, double>& uv, const Vector3D& p) const override;
    inline const Color& getColor1() const { return this->color1; }
    inline const Color& getColor2() const { return this->color2; }
    inline double getScale() const { return this->scale; }
    inline double getTurbulenceStrength() const { return this->turbulence_strength; }
    inline const std::array<int, 512>& getPerm() const { return this->perm; }
};

#endif // MARBLETEXTURE_HPP