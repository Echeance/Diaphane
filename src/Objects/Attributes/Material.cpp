#include "Material.hpp"
#include <algorithm>

Material::Material(
    const std::shared_ptr<Texture>& _albedo_texture,
    const double& _reflectivity,
    const double& _transparency,
    const double& _refractive_index,
    const double _specular_intensity,
    const double& _shininess,
    const double& _roughness,
    const double& _bump_strength,
    const std::shared_ptr<Absorption>& _absorption
) :
    albedo_texture(_albedo_texture),    
    reflectivity(std::clamp(_reflectivity, 0.0, 1.0)),
    transparency(std::clamp(_transparency, 0.0, 1.0)),
    refractive_index(_refractive_index),
    specular_intensity(std::clamp(_specular_intensity, 0.0, 1.0)),
    shininess(std::max(1.0, _shininess)),
    roughness(std::clamp(_roughness, 0.0, 1.0)),
    bump_strength(_bump_strength),
    absorption(_absorption)
{}