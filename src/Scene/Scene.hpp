#ifndef SCENE_HPP
#define SCENE_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <array>

#include <Frame/Frame.hpp>
#include <Lightning/LightSource.hpp>
#include <Maths/Vector.hpp>
#include <Objects/Object.hpp>
#include <Scene/Environment.hpp>
#include <Scene/RenderCore.hpp>

struct Viewport {
    Vector3D origin, vech, vecy;

    Viewport(Vector3D _origin = Vector3D(), Vector3D _vech = Vector3D(), Vector3D _vecy = Vector3D()) : origin(_origin), vech(_vech), vecy(_vecy) {}
};

class Scene {
private:
    Vector3D pov;
    Viewport viewport;
    Environment environment;
    short reflection_depth;

    std::vector<std::shared_ptr<LightSource>> light_sources;
    std::vector<std::shared_ptr<Object>> objects;

    struct PackedScene {
        std::vector<RtSphere> spheres;
        std::vector<RtPlane> planes;
        std::vector<RtMaterial> materials;
        std::vector<RtLight> lights;
        RtSceneData data;
        std::array<int, 512> perlin_perm{};
    };

    PackedScene pack_scene() const;

    void denoise(
        const std::vector<RtColor>& raw_color,
        const std::vector<RtVec3>& normals,
        const std::vector<RtColor>& albedos,
        Frame& out_frame,
        uint16_t width,
        uint16_t height,
        double exposure = 0.75
    ) const;

public:
    Scene(const Vector3D& _pov = Vector3D(), const Viewport& _viewport = Viewport(), const Environment& _environment = Environment::Summer(), short _reflection_depth = 5);

    inline const short& getReflectionDepth() const { return this->reflection_depth; }
    inline short& getReflectionDepth() { return this->reflection_depth; }

    void addLightSource(const std::shared_ptr<LightSource>& ls);
    void addObject(const std::shared_ptr<Object>& obj);

    Frame render(uint16_t image_width, uint16_t image_height, short samples_per_pixel = 16) const;
    Frame renderGPU(uint16_t image_width, uint16_t image_height, short samples_per_pixel = 128) const;
};

#endif // SCENE_HPP