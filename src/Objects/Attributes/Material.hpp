#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <memory>

#include <Objects/Attributes/Textures/Texture.hpp>
#include <Objects/Attributes/Textures/SolidColor.hpp>
#include <Color/Color.hpp>
#include <Color/Absorption.hpp>

class Material {
private:
    std::shared_ptr<Texture> albedo_texture;
    double reflectivity;
    double transparency;
    double refractive_index;
    double specular_intensity;
    double shininess;
    double roughness;
    double bump_strength;
    std::shared_ptr<Absorption> absorption;

public:
    Material(
        const std::shared_ptr<Texture>& _albedo_texture = std::make_shared<SolidColor>(Color{255, 0, 0}),
        const double& _reflectivity = 0.0,
        const double& _transparency = 0.0,
        const double& _refractive_index = 1.0,
        const double _specular_intensity = 0.2,
        const double& _shininess = 16.0,
        const double& _roughness = 0.0,
        const double& _bump_strength = 0.0,
        const std::shared_ptr<Absorption>& _absorption = Absorption::None()
    );

    inline const std::shared_ptr<Texture>& getTexture() const { return this->albedo_texture; }
    inline std::shared_ptr<Texture>& getTexture() { return this->albedo_texture; }

    inline const double& getReflectivity() const { return this->reflectivity; }
    inline double& getReflectivity() { return this->reflectivity; }

    inline const double& getTransparency() const { return this->transparency; }
    inline double& getTransparency() { return this->transparency; }

    inline const double& getRefractiveIndex() const { return this->refractive_index; }
    inline double& getRefractiveIndex() { return this->refractive_index; }

    inline const double& getSpecularIntensity() const { return this->specular_intensity; }
    inline double& getSpecularIntensity() { return this->specular_intensity; }

    inline const double& getShininess() const { return this->shininess; }
    inline double& getShininess() { return this->shininess; }

    inline const double& getRoughness() const { return this->roughness; }
    inline double& getRoughness() { return this->roughness; }

    inline const double& getBumpStrength() const { return this->bump_strength; }
    inline double& getBumpStrength() { return this->bump_strength; }

    inline const std::shared_ptr<Absorption>& getAbsorption() const { return this->absorption; }
    inline std::shared_ptr<Absorption>& getAbsorption() { return this->absorption; }

    // Presets avec couleur unie
    static std::shared_ptr<Material> Matte(const Color& _color = {255, 0, 0}, const double& _roughness = 0.0, const double& _bump = 0.0) {
        return std::make_shared<Material>(std::make_shared<SolidColor>(_color), 0.0, 0.0, 1.0, 0.2, 16.0, _roughness, _bump);
    }

    // Presets avec texture procédurale
    static std::shared_ptr<Material> Matte(const std::shared_ptr<Texture>& _tex, const double& _roughness = 0.0, const double& _bump = 0.0) {
        return std::make_shared<Material>(_tex, 0.0, 0.0, 1.0, 0.2, 16.0, _roughness, _bump);
    }

    static std::shared_ptr<Material> Mirror(const Color& _color = {200, 200, 200}, const double& _reflectivity = 0.9, const double& _roughness = 0.0, const double& _bump = 0.0) {
        return std::make_shared<Material>(std::make_shared<SolidColor>(_color), _reflectivity, 0.0, 1.0, 1.0, 150.0, _roughness, _bump);
    }

    static std::shared_ptr<Material> Mirror(const std::shared_ptr<Texture>& _tex, const double& _reflectivity = 0.9, const double& _roughness = 0.0, const double& _bump = 0.0) {
        return std::make_shared<Material>(_tex, _reflectivity, 0.0, 1.0, 1.0, 150.0, _roughness, _bump);
    }

    static std::shared_ptr<Material> Glass(const double& _refractive_index = 1.5, const std::shared_ptr<Absorption>& abs = Absorption::None(), const double& _bump = 0.0) {
        return std::make_shared<Material>(std::make_shared<SolidColor>(Color{255, 255, 255}), 0.1, 0.95, _refractive_index, 1.0, 250.0, 0.0, _bump, abs);
    }
};

#endif // MATERIAL_HPP