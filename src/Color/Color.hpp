#ifndef COLOR_HPP
#define COLOR_HPP

struct Color {
    double r, g, b;

    Color(const double& _r = 0.0, const double& _g = 0.0, const double& _b = 0.0);

    Color operator*(const double& scalar) const;
    Color operator*(const Color& other) const;
    friend Color operator*(const double& scalar, const Color &color);

    Color operator+(const Color& other) const;
    Color& operator+=(const Color& other);

    Color toneMap(double exposure = 1.0) const;
};

#endif // COLOR_HPP