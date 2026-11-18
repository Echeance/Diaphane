#ifndef RENDERCORE_HPP
#define RENDERCORE_HPP

#include <cmath>
#include <cstdint>

#ifdef __HIPCC__
#include <hip/hip_runtime.h>
#ifdef RT_DEVICE_ONLY
#define RT_FUNC __device__
#else
#define RT_FUNC __host__ __device__
#endif
#else
#define RT_FUNC
#endif

// ============================================================================
// Structures de données plates (POD) partagées CPU / GPU
// ============================================================================

struct RtVec3
{
    double x, y, z;
};

struct RtColor
{
    double r, g, b;
};

struct RtRay
{
    RtVec3 origin;
    RtVec3 dir;
};

struct RtSphere
{
    RtVec3 center;
    double radius;
    int material_id;
};

struct RtPlane
{
    RtVec3 normal;
    double d;
    int material_id;
};

struct RtLight
{
    RtVec3 pos;
    double intensity;
    double radius;
};

struct RtMaterial
{
    RtColor base_color;
    RtColor secondary_color;
    double reflectivity;
    double transparency;
    double refractive_index;
    double specular_intensity;
    double shininess;
    double roughness;
    double bump_strength;
    RtVec3 absorption;

    int texture_type; // 0 = Solid, 1 = Checker, 2 = Marble
    double tex_scale;
    double tex_turbulence;
    bool tex_distance_fade;
};

struct RtEnvironment
{
    RtColor horizon;
    RtColor zenith;
    bool sun_enabled;
    RtVec3 sun_dir;
    RtColor sun_color;
    double sun_intensity;
};

struct RtCamera
{
    RtVec3 pov;
    RtVec3 origin;
    RtVec3 vech;
    RtVec3 vecy;
};

struct RtSceneData
{
    const RtSphere *spheres;
    int num_spheres;
    const RtPlane *planes;
    int num_planes;
    const RtMaterial *materials;
    int num_materials;
    const RtLight *lights;
    int num_lights;
    RtCamera cam;
    RtEnvironment env;
    short max_depth;
    const int *perlin_perm;
};

struct RtHitRecord
{
    double t = 0.0;
    RtVec3 point{0.0, 0.0, 0.0};
    RtVec3 normal{0.0, 0.0, 0.0};
    int material_id = 0;
    int shape_type = 0;
    int object_idx = -1;
};

// ============================================================================
// Utilitaires mathématiques légers (inline pour inlining maximal CPU/GPU)
// ============================================================================

RT_FUNC inline double rt_min(double a, double b) { return (a < b) ? a : b; }
RT_FUNC inline double rt_max(double a, double b) { return (a > b) ? a : b; }
RT_FUNC inline double rt_clamp(double v, double lo, double hi) { return rt_min(hi, rt_max(lo, v)); }

RT_FUNC inline RtVec3 v_add(RtVec3 a, RtVec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
RT_FUNC inline RtVec3 v_sub(RtVec3 a, RtVec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
RT_FUNC inline RtVec3 v_mul(RtVec3 a, double s) { return {a.x * s, a.y * s, a.z * s}; }
RT_FUNC inline RtVec3 v_neg(RtVec3 a) { return {-a.x, -a.y, -a.z}; }
RT_FUNC inline double v_dot(RtVec3 a, RtVec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
RT_FUNC inline double v_sqlen(RtVec3 a) { return v_dot(a, a); }
RT_FUNC inline double v_len(RtVec3 a) { return std::sqrt(v_sqlen(a)); }

RT_FUNC inline RtVec3 v_norm(RtVec3 a)
{
    double l = v_len(a);
    return (l > 0.0) ? v_mul(a, 1.0 / l) : RtVec3{0.0, 0.0, 0.0};
}

RT_FUNC inline RtColor c_add(RtColor a, RtColor b) { return {a.r + b.r, a.g + b.g, a.b + b.b}; }
RT_FUNC inline RtColor c_mul_s(RtColor a, double s) { return {a.r * s, a.g * s, a.b * s}; }
RT_FUNC inline RtColor c_mul(RtColor a, RtColor b)
{
    return {(a.r * b.r) / 255.0, (a.g * b.g) / 255.0, (a.b * b.b) / 255.0};
}

// ============================================================================
// Déclarations des fonctions du moteur de rendu (définies dans RenderCore.cpp)
// ============================================================================

RT_FUNC uint32_t pcg_hash(uint32_t &state);
RT_FUNC double rt_rand(uint32_t &state);
RT_FUNC double rt_rand_range(uint32_t &state, double min_v, double max_v);
RT_FUNC RtVec3 rt_random_in_unit_sphere(uint32_t &state);

RT_FUNC double perlin_fade(double t);
RT_FUNC double perlin_lerp(double t, double a, double b);
RT_FUNC double perlin_grad(int hash, double x, double y, double z);
RT_FUNC double perlin_noise(const int *perm, RtVec3 p);
RT_FUNC double perlin_turbulence(const int *perm, RtVec3 p, int depth = 5);

RT_FUNC void get_shape_uv(int shape_type, RtVec3 center, double radius, RtVec3 p, double &out_u, double &out_v);
RT_FUNC RtColor eval_texture_color(const RtMaterial &mat, const int *perm, double u, double v, RtVec3 p);
RT_FUNC double eval_texture_height(const RtMaterial &mat, const int *perm, double u, double v, RtVec3 p);

RT_FUNC bool rt_closest_hit(const RtSceneData &scene, const RtRay &ray, RtHitRecord &hit, int ignore_obj_idx = -1);
RT_FUNC RtColor rt_obj_color(const RtSceneData &scene, const RtHitRecord &hit);
RT_FUNC RtColor rt_sky_color(const RtSceneData &scene, const RtRay &ray, bool include_sun);
RT_FUNC double rt_shadow_attenuation(const RtSceneData &scene, const RtHitRecord &hit, RtVec3 ls_dir, double dist_light);
RT_FUNC void rt_direct_lighting(
    const RtSceneData &scene,
    const RtHitRecord &hit,
    RtVec3 view_dir,
    uint32_t &rng,
    RtColor &out_diffuse,
    RtColor &out_specular);

RT_FUNC bool rt_refract(RtVec3 incident, RtVec3 normal, double refract_ratio, RtVec3 &refracted);
RT_FUNC double rt_fresnel(RtVec3 incident, RtVec3 normal, double n1, double n2);

RT_FUNC RtColor rt_trace_ray(const RtSceneData &scene, RtRay initial_ray, uint32_t &rng);
RT_FUNC void rt_render_pixel(
    uint16_t x,
    uint16_t y,
    uint16_t image_width,
    uint16_t image_height,
    short samples_per_pixel,
    const RtSceneData &scene,
    RtColor &out_raw,
    RtVec3 &out_norm,
    RtColor &out_alb);

// ============================================================================
// Point d'entrée GPU HIP
// ============================================================================

void launch_hip_render(
    uint16_t image_width,
    uint16_t image_height,
    short samples_per_pixel,
    const RtSceneData &host_scene,
    RtColor *out_raw,
    RtVec3 *out_norm,
    RtColor *out_alb);

#endif // RENDERCORE_HPP