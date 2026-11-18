#ifndef BALL_HPP
#define BALL_HPP

#include <Objects/Object.hpp>
#include <Color/Color.hpp>

class Ball : public Object {
public:
    Ball(const Vector3D& _center, double _radius = .0, const std::shared_ptr<Material>& _material = Material::Matte());
};

#endif // BALL_HPP