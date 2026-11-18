#ifndef ABSORPTION_HPP
#define ABSORPTION_HPP

#include <memory>

#include <Maths/Vector.hpp>

class Absorption {
private:
    Vector3D components;

public:
    Absorption(const double& red_ratio = 0.0, const double& green_ratio = 0.0, const double& blue_ratio = 0.0);

    inline const double& getRed() const { return this->components.getX(); };
    inline double& getRed() { return this->components.getX(); };

    inline const double& getGreen() const { return this->components.getY(); };
    inline double& getGreen() { return this->components.getY(); };

    inline const double& getBlue() const { return this->components.getZ(); };
    inline double& getBlue() { return this->components.getZ(); };

    static std::shared_ptr<Absorption> None() {
        return std::make_shared<Absorption>(0.0, 0.0, 0.0);
    }

    static std::shared_ptr<Absorption> Emerald() {
        return std::make_shared<Absorption>(1.5, 0.1, 1.5);
    }

    static std::shared_ptr<Absorption> Saphire() {
        return std::make_shared<Absorption>(1.2, 0.4, 0.05);
    }

    static std::shared_ptr<Absorption> Amber() {
        return std::make_shared<Absorption>(0.1, 0.6, 2.0);
    }
};

#endif // ABSORPTION_HPP