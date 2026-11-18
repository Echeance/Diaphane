#ifndef FLOOR_HPP
#define FLOOR_HPP

#include <Objects/Object.hpp>

class Floor : public Object {
public:
    // elevation représente la hauteur du sol sur l'axe Y (ex: -1.0)
    Floor(const Vector3D& _center, double elevation = 0.0);
};

#endif // FLOOR_HPP