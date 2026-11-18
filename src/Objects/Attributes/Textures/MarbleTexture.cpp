#include "MarbleTexture.hpp"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

MarbleTexture::MarbleTexture(const Color& _color1, const Color& _color2, double _scale, double _turbulence_strength) :
    color1(_color1),
    color2(_color2),
    scale(_scale),
    turbulence_strength(_turbulence_strength)
{
    std::array<int, 256> p;
    std::iota(p.begin(), p.end(), 0);

    std::mt19937 gen(1337);
    std::shuffle(p.begin(), p.end(), gen);

    for (int i = 0; i < 256; ++i) {
        this->perm[i] = p[i];
        this->perm[256 + i] = p[i];
    }
}

double MarbleTexture::fade(double t) const {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

double MarbleTexture::lerp(double t, double a, double b) const {
    return a + t * (b - a);
}

double MarbleTexture::grad(int hash, double x, double y, double z) const {
    int h = hash & 15;
    double u = (h < 8) ? x : y;
    double v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);
    return (((h & 1) == 0) ? u : -u) + (((h & 2) == 0) ? v : -v);
}

double MarbleTexture::noise(const Vector3D& p) const {
    int X = static_cast<int>(std::floor(p.getX())) & 255;
    int Y = static_cast<int>(std::floor(p.getY())) & 255;
    int Z = static_cast<int>(std::floor(p.getZ())) & 255;

    double x = p.getX() - std::floor(p.getX());
    double y = p.getY() - std::floor(p.getY());
    double z = p.getZ() - std::floor(p.getZ());

    double u = this->fade(x);
    double v = this->fade(y);
    double w = this->fade(z);

    int A  = this->perm[X] + Y;
    int AA = this->perm[A] + Z;
    int AB = this->perm[A + 1] + Z;
    int B  = this->perm[X + 1] + Y;
    int BA = this->perm[B] + Z;
    int BB = this->perm[B + 1] + Z;

    return this->lerp(w,
        this->lerp(v,
            this->lerp(u, this->grad(this->perm[AA], x, y, z),     this->grad(this->perm[BA], x - 1, y, z)),
            this->lerp(u, this->grad(this->perm[AB], x, y - 1, z), this->grad(this->perm[BB], x - 1, y - 1, z))
        ),
        this->lerp(v,
            this->lerp(u, this->grad(this->perm[AA + 1], x, y, z - 1),     this->grad(this->perm[BA + 1], x - 1, y, z - 1)),
            this->lerp(u, this->grad(this->perm[AB + 1], x, y - 1, z - 1), this->grad(this->perm[BB + 1], x - 1, y - 1, z - 1))
        )
    );
}

double MarbleTexture::turbulence(const Vector3D& p, int depth) const {
    double accum = 0.0;
    Vector3D temp_p = p;
    double weight = 1.0;

    for (int i = 0; i < depth; ++i) {
        accum += weight * std::fabs(this->noise(temp_p));
        weight *= 0.5;
        temp_p = temp_p * 2.0;
    }

    return accum;
}

Color MarbleTexture::value(const std::pair<double, double>&, const Vector3D& p) const {
    double turb = this->turbulence(p * this->scale);
    double t = 0.5 * (1.0 + std::sin(this->scale * p.getX() + this->turbulence_strength * turb));

    return Color(
        (1.0 - t) * this->color1.r + t * this->color2.r,
        (1.0 - t) * this->color1.g + t * this->color2.g,
        (1.0 - t) * this->color1.b + t * this->color2.b
    );
}

double MarbleTexture::height(const std::pair<double, double>&, const Vector3D& p) const {
    return this->turbulence(p * this->scale);
}