#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <memory>

#include <Color/Color.hpp>
#include <Lightning/LightSource.hpp>
#include <Objects/Attributes/Shapes/Shape.hpp>
#include <Objects/Attributes/Material.hpp>

class Object {
protected:
    Vector3D center;
    std::shared_ptr<Shape> shape;
    std::shared_ptr<Material> material;

public:
    Object(const Vector3D& _center, std::shared_ptr<Shape> _shape, const std::shared_ptr<Material>& _material = Material::Matte());
    virtual ~Object() = default;

    inline const Vector3D& getCenter() const { return this->center; }
    inline Vector3D& getCenter() { return this->center; }

    inline const std::shared_ptr<Shape>& getShape() const { return this->shape; }
    inline std::shared_ptr<Shape>& getShape() { return this->shape; }

    virtual Color getColor(const Vector3D& impact_point = Vector3D()) const { 
        return this->material->getTexture()->value(this->shape->get_uv(this->center, impact_point), impact_point); 
    }

    inline const std::shared_ptr<Material>& getMaterial() const { return this->material; }
    inline std::shared_ptr<Material>& getMaterial() { return this->material; }

    inline Vector3D normal_at(const Vector3D& impact_point) const { return this->shape->normal_at(this->center, impact_point); };
    inline double intersect(const Ray& ray) const { return this->shape->intersect(this->center, ray); };
};

#endif // OBJECT_HPP