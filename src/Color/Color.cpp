#include "Color.hpp"

#include <cmath>
#include <algorithm>

Color::Color(const double& _r, const double& _g, const double& _b) : r(_r), g(_g), b(_b) {};

Color Color::operator*(const double& scalar) const {
    return Color(r * scalar, g * scalar, b * scalar);
}

Color operator*(const double& scalar, const Color& color) {
    return Color(color.r * scalar, color.g * scalar, color.b * scalar);
}

Color Color::operator*(const Color& other) const {
    return Color((r * other.r) / 255.0, (g * other.g) / 255.0, (b * other.b) / 255.0);
}

Color Color::operator+(const Color& other) const {
    return Color(r + other.r, g + other.g, b + other.b);
}

Color& Color::operator+=(const Color& other) {
    r += other.r;
    g += other.g;
    b += other.b;
    return *this;
}

Color Color::toneMap(double exposure) const {
    // On normalise sur [0, +inf[ (255.0 = 1.0)
    double nr = std::max(0.0, (r / 255.0) * exposure);
    double ng = std::max(0.0, (g / 255.0) * exposure);
    double nb = std::max(0.0, (b / 255.0) * exposure);

    // Correction Gamma (2.2) pour un rendu lumineux et contrasté sur écran standard

    // const double inv_gamma = 1.0 / 2.2;
    
    // Courbe ACES Filmic (contraste cinéma, pas de couleurs délavées)
    auto aces = [](double x) {
        return std::clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
    };

    // nr = std::pow(aces(nr), inv_gamma);
    // ng = std::pow(aces(ng), inv_gamma);
    // nb = std::pow(aces(nb), inv_gamma);

    // Reconversion en 0 - 255 pour l'affichage / fichier PPM
    return Color(
        std::round(aces(nr) * 255.0),
        std::round(aces(ng) * 255.0),
        std::round(aces(nb) * 255.0)
    );
}