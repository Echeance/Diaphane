#include "CheckerTexture.hpp"
#include <cmath>
#include <algorithm>

CheckerTexture::CheckerTexture(const Color& _even, const Color& _odd, double _scale, bool _distance_fade) :
    even(std::make_shared<SolidColor>(_even)),
    odd(std::make_shared<SolidColor>(_odd)),
    scale(_scale),
    distance_fade(_distance_fade)
{}

CheckerTexture::CheckerTexture(const std::shared_ptr<Texture>& _even, const std::shared_ptr<Texture>& _odd, double _scale, bool _distance_fade) :
    even(_even),
    odd(_odd),
    scale(_scale),
    distance_fade(_distance_fade)
{}

Color CheckerTexture::value(const std::pair<double, double>& uv, const Vector3D& p) const {
    int checker = static_cast<int>(std::floor(uv.first * this->scale) + std::floor(uv.second * this->scale));
    Color base = (checker % 2 == 0) ? this->even->value(uv, p) : this->odd->value(uv, p);

    if (this->distance_fade) {
        double dist = p.length();
        double factor = std::clamp(1.0 / (1.0 + dist * 0.05), 0.2, 1.0);
        return base * factor;
    }

    return base;
}

double CheckerTexture::height(const std::pair<double, double>& uv, const Vector3D&) const {
    int checker = static_cast<int>(std::floor(uv.first * this->scale) + std::floor(uv.second * this->scale));
    return (checker % 2 == 0) ? 1.0 : 0.0;
}