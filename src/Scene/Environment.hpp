#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <Color/Color.hpp>
#include <Scene/Sun.hpp>

class Environment {
private:
    Color horizon, zenith;
    Sun sun;

public:
    Environment(const Color& _horizon, const Color& _zenith, const Sun& _sun = Sun());

    inline const Color& getHorizon() const { return this->horizon; }
    inline Color& getHorizon() { return this->horizon; }

    inline const Color& getZenith() const { return this->zenith; }
    inline Color& getZenith() { return this->zenith; }

    inline const Sun& getSun() const { return this->sun; }
    inline Sun& getSun() { return this->sun; }

    static Environment Summer() {
        return Environment(Color{210, 230, 250}, Color{50, 110, 210}, Sun(Vector3D(-2.0, 2.5, -1.5), Color{255, 250, 230}, 1.2));
    }

    static Environment Arctic() {
        return Environment(Color{240, 245, 255}, Color{100, 150, 230}, Sun(Vector3D(-2.0, 2.5, -1.5), Color{255, 250, 230}, 1.2));
    }

    static Environment Sunset() {
        return Environment(Color{255, 160, 90}, Color{45, 65, 130}, Sun(Vector3D(-2.0, 2.5, -1.5), Color{255, 250, 230}, 1.2));
    }

    static Environment Cloudy() {
        return Environment(Color{225, 228, 232}, Color{140, 150, 165}, Sun(Vector3D(-2.0, 2.5, -1.5), Color{255, 250, 230}, 1.2));
    }
};

#endif // ENVIRONMENT_HPP