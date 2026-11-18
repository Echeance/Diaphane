#include "RenderCore.hpp"

// ============================================================================
// Générateur aléatoire PCG déterministe identique CPU / GPU
// ============================================================================

RT_FUNC uint32_t pcg_hash(uint32_t &state)
{
    state = state * 747796405u + 2891336453u;
    uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

RT_FUNC double rt_rand(uint32_t &state)
{
    return static_cast<double>(pcg_hash(state)) / 4294967296.0;
}

RT_FUNC double rt_rand_range(uint32_t &state, double min_v, double max_v)
{
    return min_v + (max_v - min_v) * rt_rand(state);
}

RT_FUNC RtVec3 rt_random_in_unit_sphere(uint32_t &state)
{
    RtVec3 u;
    do
    {
        u.x = rt_rand_range(state, -1.0, 1.0);
        u.y = rt_rand_range(state, -1.0, 1.0);
        u.z = rt_rand_range(state, -1.0, 1.0);
    } while (v_sqlen(u) >= 1.0);
    return u;
}

// ============================================================================
// Perlin 3D & Textures exactes (couleur + hauteur pour bump mapping)
// ============================================================================

RT_FUNC double perlin_fade(double t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

RT_FUNC double perlin_lerp(double t, double a, double b)
{
    return a + t * (b - a);
}

RT_FUNC double perlin_grad(int hash, double x, double y, double z)
{
    int h = hash & 15;
    double u = (h < 8) ? x : y;
    double v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);
    return (((h & 1) == 0) ? u : -u) + (((h & 2) == 0) ? v : -v);
}

RT_FUNC double perlin_noise(const int *perm, RtVec3 p)
{
    int X = static_cast<int>(std::floor(p.x)) & 255;
    int Y = static_cast<int>(std::floor(p.y)) & 255;
    int Z = static_cast<int>(std::floor(p.z)) & 255;

    double x = p.x - std::floor(p.x);
    double y = p.y - std::floor(p.y);
    double z = p.z - std::floor(p.z);

    double u = perlin_fade(x);
    double v = perlin_fade(y);
    double w = perlin_fade(z);

    int A = perm[X] + Y;
    int AA = perm[A] + Z;
    int AB = perm[A + 1] + Z;
    int B = perm[X + 1] + Y;
    int BA = perm[B] + Z;
    int BB = perm[B + 1] + Z;

    return perlin_lerp(w,
                       perlin_lerp(v,
                                   perlin_lerp(u, perlin_grad(perm[AA], x, y, z), perlin_grad(perm[BA], x - 1, y, z)),
                                   perlin_lerp(u, perlin_grad(perm[AB], x, y - 1, z), perlin_grad(perm[BB], x - 1, y - 1, z))),
                       perlin_lerp(v,
                                   perlin_lerp(u, perlin_grad(perm[AA + 1], x, y, z - 1), perlin_grad(perm[BA + 1], x - 1, y, z - 1)),
                                   perlin_lerp(u, perlin_grad(perm[AB + 1], x, y - 1, z - 1), perlin_grad(perm[BB + 1], x - 1, y - 1, z - 1))));
}

RT_FUNC double perlin_turbulence(const int *perm, RtVec3 p, int depth)
{
    double accum = 0.0;
    RtVec3 temp_p = p;
    double weight = 1.0;

    for (int i = 0; i < depth; ++i)
    {
        accum += weight * std::fabs(perlin_noise(perm, temp_p));
        weight *= 0.5;
        temp_p = v_mul(temp_p, 2.0);
    }
    return accum;
}

RT_FUNC void get_shape_uv(int shape_type, RtVec3 center, double radius, RtVec3 p, double &out_u, double &out_v)
{
    if (shape_type == 0)
    { // Sphere
        RtVec3 n = v_mul(v_sub(p, center), 1.0 / radius);
        double theta = std::atan2(n.z, n.x);
        double phi = std::asin(rt_clamp(n.y, -1.0, 1.0));
        out_u = 0.5 + theta / (2.0 * M_PI);
        out_v = 0.5 + phi / M_PI;
    }
    else
    { // Plane
        out_u = p.x;
        out_v = p.z;
    }
}

RT_FUNC RtColor eval_texture_color(const RtMaterial &mat, const int *perm, double u, double v, RtVec3 p)
{
    if (mat.texture_type == 1)
    { // CheckerTexture
        int checker = static_cast<int>(std::floor(u * mat.tex_scale) + std::floor(v * mat.tex_scale));
        RtColor base = (checker % 2 == 0) ? mat.base_color : mat.secondary_color;
        if (mat.tex_distance_fade)
        {
            double factor = rt_clamp(1.0 / (1.0 + v_len(p) * 0.05), 0.2, 1.0);
            return c_mul_s(base, factor);
        }
        return base;
    }
    if (mat.texture_type == 2)
    { // MarbleTexture
        double turb = perlin_turbulence(perm, v_mul(p, mat.tex_scale));
        double t = 0.5 * (1.0 + std::sin(mat.tex_scale * p.x + mat.tex_turbulence * turb));
        return {
            (1.0 - t) * mat.base_color.r + t * mat.secondary_color.r,
            (1.0 - t) * mat.base_color.g + t * mat.secondary_color.g,
            (1.0 - t) * mat.base_color.b + t * mat.secondary_color.b};
    }
    return mat.base_color;
}

RT_FUNC double eval_texture_height(const RtMaterial &mat, const int *perm, double u, double v, RtVec3 p)
{
    if (mat.texture_type == 1)
    {
        int checker = static_cast<int>(std::floor(u * mat.tex_scale) + std::floor(v * mat.tex_scale));
        return (checker % 2 == 0) ? 1.0 : 0.0;
    }
    if (mat.texture_type == 2)
    {
        return perlin_turbulence(perm, v_mul(p, mat.tex_scale));
    }
    return 0.0;
}

// ============================================================================
// Intersections, Bump Mapping & Éclairage
// ============================================================================

RT_FUNC bool rt_closest_hit(const RtSceneData &scene, const RtRay &ray, RtHitRecord &hit, int ignore_obj_idx)
{
    constexpr double EPS = 1e-3;
    double min_t = 1e30;
    bool found = false;

    int hit_shape_type = 0;
    int hit_shape_idx = -1;
    RtVec3 raw_normal{0.0, 0.0, 0.0};

    for (int i = 0; i < scene.num_spheres; ++i)
    {
        int global_idx = i;
        if (global_idx == ignore_obj_idx)
            continue;

        RtVec3 oc = v_sub(ray.origin, scene.spheres[i].center);
        double a = v_dot(ray.dir, ray.dir);
        double b = 2.0 * v_dot(ray.dir, oc);
        double c = v_dot(oc, oc) - scene.spheres[i].radius * scene.spheres[i].radius;
        double disc = b * b - 4.0 * a * c;

        if (disc >= 0.0)
        {
            double sq = std::sqrt(disc);
            double t1 = (-b - sq) / (2.0 * a);
            double t2 = (-b + sq) / (2.0 * a);
            double t = (t1 > EPS) ? t1 : ((t2 > EPS) ? t2 : -1.0);

            if (t > EPS && t < min_t)
            {
                min_t = t;
                hit.t = t;
                hit.point = v_add(ray.origin, v_mul(ray.dir, t));
                raw_normal = v_mul(v_sub(hit.point, scene.spheres[i].center), 1.0 / scene.spheres[i].radius);
                hit.material_id = scene.spheres[i].material_id;
                hit.object_idx = global_idx;
                hit_shape_type = 0;
                hit_shape_idx = i;
                found = true;
            }
        }
    }

    for (int i = 0; i < scene.num_planes; ++i)
    {
        int global_idx = scene.num_spheres + i;
        if (global_idx == ignore_obj_idx)
            continue;

        double denom = v_dot(scene.planes[i].normal, ray.dir);
        if (std::fabs(denom) >= 1e-6)
        {
            double t = -(v_dot(scene.planes[i].normal, ray.origin) + scene.planes[i].d) / denom;
            if (t > EPS && t < min_t)
            {
                min_t = t;
                hit.t = t;
                hit.point = v_add(ray.origin, v_mul(ray.dir, t));
                raw_normal = scene.planes[i].normal;
                hit.material_id = scene.planes[i].material_id;
                hit.object_idx = global_idx;
                hit_shape_type = 1;
                hit_shape_idx = i;
                found = true;
            }
        }
    }

    if (!found)
        return false;

    hit.shape_type = hit_shape_type;
    hit.normal = raw_normal;

    const RtMaterial &mat = scene.materials[hit.material_id];
    if (mat.bump_strength != 0.0)
    {
        constexpr double delta = 1e-4;
        RtVec3 center = (hit_shape_type == 0) ? scene.spheres[hit_shape_idx].center : RtVec3{0.0, 0.0, 0.0};
        double radius = (hit_shape_type == 0) ? scene.spheres[hit_shape_idx].radius : 1.0;

        auto sample_h = [&](RtVec3 p)
        {
            double u, v;
            get_shape_uv(hit_shape_type, center, radius, p, u, v);
            return eval_texture_height(mat, scene.perlin_perm, u, v, p);
        };

        double dh_dx = (sample_h(v_add(hit.point, {delta, 0.0, 0.0})) - sample_h(v_sub(hit.point, {delta, 0.0, 0.0}))) / (2.0 * delta);
        double dh_dy = (sample_h(v_add(hit.point, {0.0, delta, 0.0})) - sample_h(v_sub(hit.point, {0.0, delta, 0.0}))) / (2.0 * delta);
        double dh_dz = (sample_h(v_add(hit.point, {0.0, 0.0, delta})) - sample_h(v_sub(hit.point, {0.0, 0.0, delta}))) / (2.0 * delta);

        hit.normal = v_norm(v_sub(hit.normal, v_mul({dh_dx, dh_dy, dh_dz}, mat.bump_strength)));
    }

    return true;
}

RT_FUNC RtColor rt_obj_color(const RtSceneData &scene, const RtHitRecord &hit)
{
    const RtMaterial &mat = scene.materials[hit.material_id];
    double u = 0.0, v = 0.0;
    if (hit.shape_type == 0)
    {
        int s_idx = hit.object_idx;
        get_shape_uv(0, scene.spheres[s_idx].center, scene.spheres[s_idx].radius, hit.point, u, v);
    }
    else
    {
        get_shape_uv(1, {0.0, 0.0, 0.0}, 1.0, hit.point, u, v);
    }
    return eval_texture_color(mat, scene.perlin_perm, u, v, hit.point);
}

RT_FUNC RtColor rt_sky_color(const RtSceneData &scene, const RtRay &ray, bool include_sun)
{
    double t = std::pow(rt_clamp(ray.dir.y, 0.0, 1.0), 0.6);
    RtColor sky{
        (1.0 - t) * scene.env.horizon.r + t * scene.env.zenith.r,
        (1.0 - t) * scene.env.horizon.g + t * scene.env.zenith.g,
        (1.0 - t) * scene.env.horizon.b + t * scene.env.zenith.b};

    if (include_sun && scene.env.sun_enabled)
    {
        double sun_alignment = rt_max(0.0, v_dot(ray.dir, scene.env.sun_dir));
        double halo = std::pow(sun_alignment, 32.0) * 0.4 * scene.env.sun_intensity;
        double disc = std::pow(sun_alignment, 1000.0) * 40.0 * scene.env.sun_intensity;
        sky = c_add(sky, c_mul_s({255.0, 240.0, 200.0}, halo));
        sky = c_add(sky, c_mul_s(scene.env.sun_color, disc));
    }
    return sky;
}

RT_FUNC double rt_shadow_attenuation(const RtSceneData &scene, const RtHitRecord &hit, RtVec3 ls_dir, double dist_light)
{
    constexpr double EPS = 1e-3;
    RtRay shadow_ray{v_add(hit.point, v_mul(hit.normal, EPS)), ls_dir};
    double attenuation = 1.0;

    for (int i = 0; i < scene.num_spheres; ++i)
    {
        if (i == hit.object_idx)
            continue;
        RtVec3 oc = v_sub(shadow_ray.origin, scene.spheres[i].center);
        double a = v_dot(shadow_ray.dir, shadow_ray.dir);
        double b = 2.0 * v_dot(shadow_ray.dir, oc);
        double c = v_dot(oc, oc) - scene.spheres[i].radius * scene.spheres[i].radius;
        double disc = b * b - 4.0 * a * c;
        if (disc >= 0.0)
        {
            double sq = std::sqrt(disc);
            double t1 = (-b - sq) / (2.0 * a);
            double t2 = (-b + sq) / (2.0 * a);
            double t = (t1 > EPS) ? t1 : ((t2 > EPS) ? t2 : -1.0);
            if (t > EPS && t < dist_light)
            {
                const RtMaterial &mat = scene.materials[scene.spheres[i].material_id];
                if (mat.transparency <= 0.0)
                    return 0.0;
                attenuation = rt_clamp(attenuation * (mat.transparency / mat.refractive_index), 0.0, 1.0);
            }
        }
    }

    for (int i = 0; i < scene.num_planes; ++i)
    {
        int global_idx = scene.num_spheres + i;
        if (global_idx == hit.object_idx)
            continue;
        double denom = v_dot(scene.planes[i].normal, shadow_ray.dir);
        if (std::fabs(denom) >= 1e-6)
        {
            double t = -(v_dot(scene.planes[i].normal, shadow_ray.origin) + scene.planes[i].d) / denom;
            if (t > EPS && t < dist_light)
            {
                const RtMaterial &mat = scene.materials[scene.planes[i].material_id];
                if (mat.transparency <= 0.0)
                    return 0.0;
                attenuation = rt_clamp(attenuation * (mat.transparency / mat.refractive_index), 0.0, 1.0);
            }
        }
    }

    return attenuation;
}

RT_FUNC void rt_direct_lighting(
    const RtSceneData &scene,
    const RtHitRecord &hit,
    RtVec3 view_dir,
    uint32_t &rng,
    RtColor &out_diffuse,
    RtColor &out_specular)
{
    double total_diffuse = 0.0;
    double total_specular = 0.0;
    const RtMaterial &mat = scene.materials[hit.material_id];

    for (int l = 0; l < scene.num_lights; ++l)
    {
        RtVec3 target = scene.lights[l].pos;
        if (scene.lights[l].radius > 0.0)
        {
            target = v_add(target, v_mul(rt_random_in_unit_sphere(rng), scene.lights[l].radius));
        }

        RtVec3 ls_vec = v_sub(target, hit.point);
        double dist_light = v_len(ls_vec);
        RtVec3 ls_dir = v_mul(ls_vec, 1.0 / dist_light);

        double attenuation = rt_shadow_attenuation(scene, hit, ls_dir, dist_light);
        if (attenuation <= 0.0)
            continue;

        double diff_factor = rt_max(0.0, v_dot(hit.normal, ls_dir));
        total_diffuse += diff_factor * scene.lights[l].intensity * attenuation;

        RtVec3 median_vec = v_norm(v_add(ls_dir, view_dir));
        double spec_angle = rt_max(0.0, v_dot(hit.normal, median_vec));
        double effective_shininess = rt_max(2.0, mat.shininess * (1.0 - mat.roughness));
        total_specular += std::pow(spec_angle, effective_shininess) * mat.specular_intensity * scene.lights[l].intensity * attenuation;
    }

    total_diffuse = rt_min(1.0, total_diffuse);
    out_diffuse = c_mul_s(rt_obj_color(scene, hit), total_diffuse);
    out_specular = c_mul_s({255.0, 255.0, 255.0}, total_specular);
}

RT_FUNC bool rt_refract(RtVec3 incident, RtVec3 normal, double refract_ratio, RtVec3 &refracted)
{
    double cos_theta = rt_min(v_dot(v_neg(incident), normal), 1.0);
    RtVec3 r_out_perp = v_mul(v_add(incident, v_mul(normal, cos_theta)), refract_ratio);
    double parallel_term_sq = 1.0 - v_sqlen(r_out_perp);
    if (parallel_term_sq < 0.0)
        return false;
    RtVec3 r_out_parallel = v_mul(normal, -std::sqrt(parallel_term_sq));
    refracted = v_norm(v_add(r_out_perp, r_out_parallel));
    return true;
}

RT_FUNC double rt_fresnel(RtVec3 incident, RtVec3 normal, double n1, double n2)
{
    double cos_theta = rt_min(v_dot(v_neg(incident), normal), 1.0);
    double r0 = (n1 - n2) / (n1 + n2);
    r0 *= r0;
    return r0 + (1.0 - r0) * std::pow(1.0 - cos_theta, 5.0);
}

// ============================================================================
// Path Tracer exact
// ============================================================================

RT_FUNC RtColor rt_trace_ray(const RtSceneData &scene, RtRay initial_ray, uint32_t &rng)
{
    constexpr double EPS = 1e-3;
    constexpr int MAX_STACK = 6;

    struct RayTask
    {
        RtRay ray{};
        short depth = 0;
        bool is_diffuse_ray = false;
        RtColor weight{0.0, 0.0, 0.0};
    };

    RayTask stack[MAX_STACK];
    int stack_ptr = 0;

    stack[stack_ptr++] = {initial_ray, scene.max_depth, false, {255.0, 255.0, 255.0}};
    RtColor total_radiance{0.0, 0.0, 0.0};

    while (stack_ptr > 0)
    {
        RayTask task = stack[--stack_ptr];
        RtHitRecord hit;

        if (!rt_closest_hit(scene, task.ray, hit))
        {
            RtColor sky = rt_sky_color(scene, task.ray, !task.is_diffuse_ray);
            total_radiance = c_add(total_radiance, c_mul(task.weight, sky));
            continue;
        }

        const RtMaterial &mat = scene.materials[hit.material_id];
        RtColor albedo = rt_obj_color(scene, hit);

        bool is_exiting = (v_dot(task.ray.dir, hit.normal) > 0.0);
        RtColor seg_weight = task.weight;
        if (is_exiting && mat.transparency > 0.0)
        {
            seg_weight.r *= std::exp(-mat.absorption.x * hit.t);
            seg_weight.g *= std::exp(-mat.absorption.y * hit.t);
            seg_weight.b *= std::exp(-mat.absorption.z * hit.t);
        }

        RtColor diffuse_color{0.0, 0.0, 0.0};
        RtColor specular_color{0.0, 0.0, 0.0};
        rt_direct_lighting(scene, hit, v_neg(task.ray.dir), rng, diffuse_color, specular_color);

        total_radiance = c_add(total_radiance, c_mul(seg_weight, specular_color));

        double diffuse_scale = 1.0;
        bool has_explicit_secondary = (mat.reflectivity > 0.0 || mat.transparency > 0.0);
        bool has_fresnel_roughness = (mat.roughness > 0.0 && !task.is_diffuse_ray);

        bool is_entering = (v_dot(task.ray.dir, hit.normal) < 0.0);
        RtVec3 outward_normal = is_entering ? hit.normal : v_neg(hit.normal);

        if ((has_explicit_secondary || has_fresnel_roughness) && task.depth > 0)
        {
            if (mat.transparency > 0.0)
            {
                diffuse_scale = 1.0 - mat.transparency;
            }
            else if (mat.reflectivity > 0.0)
            {
                diffuse_scale = 1.0 - mat.reflectivity;
            }
            else
            {
                double fresnel_R = rt_fresnel(task.ray.dir, outward_normal, 1.0, 1.5);
                diffuse_scale = 1.0 - fresnel_R;
            }
        }

        total_radiance = c_add(total_radiance, c_mul(seg_weight, c_mul_s(diffuse_color, diffuse_scale)));

        if (mat.reflectivity < 1.0 && mat.transparency < 1.0 && task.depth > 0 && diffuse_scale > 0.0)
        {
            bool should_bounce = true;
            double rr_weight = 1.0;

            if (task.depth <= scene.max_depth - 2)
            {
                double max_a = rt_max(albedo.r, rt_max(albedo.g, albedo.b)) / 255.0;
                double survival_prob = rt_clamp(max_a, 0.35, 0.95);
                if (rt_rand(rng) <= survival_prob)
                    rr_weight /= survival_prob;
                else
                    should_bounce = false;
            }

            if (should_bounce && stack_ptr < MAX_STACK)
            {
                RtVec3 u = rt_random_in_unit_sphere(rng);
                while (v_sqlen(u) <= 1e-8)
                    u = rt_random_in_unit_sphere(rng);

                RtVec3 diffused_dir = v_add(outward_normal, v_norm(u));
                diffused_dir = (v_sqlen(diffused_dir) < 1e-8) ? outward_normal : v_norm(diffused_dir);

                RtColor bounce_w = c_mul(seg_weight, c_mul_s(albedo, rr_weight * diffuse_scale));
                stack[stack_ptr++] = {
                    {v_add(hit.point, v_mul(outward_normal, EPS)), diffused_dir},
                    static_cast<short>(task.depth - 1),
                    true,
                    bounce_w};
            }
        }

        if ((has_explicit_secondary || has_fresnel_roughness) && task.depth > 0)
        {
            double n1 = is_entering ? 1.0 : mat.refractive_index;
            double n2 = is_entering ? mat.refractive_index : 1.0;

            RtVec3 ideal_refl = v_sub(task.ray.dir, v_mul(outward_normal, 2.0 * v_dot(task.ray.dir, outward_normal)));
            RtVec3 refl_dir = ideal_refl;
            if (mat.roughness > 0.0)
            {
                do
                {
                    refl_dir = v_norm(v_add(ideal_refl, v_mul(rt_random_in_unit_sphere(rng), mat.roughness)));
                } while (v_dot(refl_dir, outward_normal) <= 0.0);
            }
            RtRay refl_ray{v_add(hit.point, v_mul(outward_normal, EPS)), refl_dir};

            if (mat.transparency > 0.0)
            {
                RtVec3 ideal_refr;
                double fresnel_R = 1.0;
                bool can_refract = rt_refract(task.ray.dir, outward_normal, n1 / n2, ideal_refr);

                if (can_refract)
                {
                    fresnel_R = rt_fresnel(task.ray.dir, outward_normal, n1, n2);
                    RtVec3 refr_dir = ideal_refr;
                    if (mat.roughness > 0.0)
                    {
                        do
                        {
                            refr_dir = v_norm(v_add(ideal_refr, v_mul(rt_random_in_unit_sphere(rng), mat.roughness)));
                        } while (v_dot(refr_dir, outward_normal) >= 0.0);
                    }

                    if (stack_ptr < MAX_STACK)
                    {
                        RtColor refr_w = c_mul_s(seg_weight, mat.transparency * (1.0 - fresnel_R));
                        stack[stack_ptr++] = {
                            {v_sub(hit.point, v_mul(outward_normal, EPS)), refr_dir},
                            static_cast<short>(task.depth - 1),
                            false,
                            refr_w};
                    }
                }

                if (stack_ptr < MAX_STACK)
                {
                    RtColor refl_w = c_mul_s(seg_weight, mat.transparency * fresnel_R);
                    stack[stack_ptr++] = {refl_ray, static_cast<short>(task.depth - 1), false, refl_w};
                }
            }
            else if (mat.reflectivity > 0.0)
            {
                if (stack_ptr < MAX_STACK)
                {
                    RtColor refl_w = c_mul(seg_weight, c_mul_s(albedo, mat.reflectivity));
                    stack[stack_ptr++] = {refl_ray, static_cast<short>(task.depth - 1), false, refl_w};
                }
            }
            else
            {
                double fresnel_R = rt_fresnel(task.ray.dir, outward_normal, 1.0, 1.5);
                if (stack_ptr < MAX_STACK)
                {
                    RtColor refl_w = c_mul_s(seg_weight, fresnel_R);
                    stack[stack_ptr++] = {refl_ray, static_cast<short>(task.depth - 1), false, refl_w};
                }
            }
        }
    }

    return total_radiance;
}

// ============================================================================
// Rendu d'un pixel complet (appelé par les threads CPU ET par le kernel GPU)
// ============================================================================

RT_FUNC void rt_render_pixel(
    uint16_t x,
    uint16_t y,
    uint16_t image_width,
    uint16_t image_height,
    short samples_per_pixel,
    const RtSceneData &scene,
    RtColor &out_raw,
    RtVec3 &out_norm,
    RtColor &out_alb)
{
    constexpr double EPS = 1e-3;
    size_t idx = static_cast<size_t>(y) * image_width + x;
    uint32_t rng = static_cast<uint32_t>(idx * 1973u + 9277u);

    RtColor sum_c{0.0, 0.0, 0.0};
    RtVec3 sum_n{0.0, 0.0, 0.0};
    RtColor sum_a{0.0, 0.0, 0.0};

    for (short s = 0; s < samples_per_pixel; ++s)
    {
        double u = (x + rt_rand(rng)) / (image_width - 1);
        double v = (image_height - 1 - (y + rt_rand(rng))) / (image_height - 1);

        RtVec3 pixel_pos = v_add(scene.cam.origin, v_add(v_mul(scene.cam.vech, u), v_mul(scene.cam.vecy, v)));
        RtRay r{scene.cam.pov, v_norm(v_sub(pixel_pos, scene.cam.pov))};

        // Recherche du premier impact diffus/rugueux pour un G-Buffer net à travers miroirs et verre
        RtRay g_ray = r;
        for (int bounce = 0; bounce < 4; ++bounce)
        {
            RtHitRecord g_hit{};
            if (!rt_closest_hit(scene, g_ray, g_hit))
            {
                break;
            }

            const RtMaterial &mat = scene.materials[g_hit.material_id];
            bool is_smooth_mirror = (mat.reflectivity > 0.5 && mat.roughness < 0.05 && mat.transparency <= 0.0);
            bool is_smooth_glass = (mat.transparency > 0.5 && mat.roughness < 0.05);

            if (!is_smooth_mirror && !is_smooth_glass)
            {
                sum_n = v_add(sum_n, g_hit.normal);
                sum_a = c_add(sum_a, rt_obj_color(scene, g_hit));
                break;
            }

            bool is_entering = (v_dot(g_ray.dir, g_hit.normal) < 0.0);
            RtVec3 out_n = is_entering ? g_hit.normal : v_neg(g_hit.normal);

            if (is_smooth_glass)
            {
                double n1 = is_entering ? 1.0 : mat.refractive_index;
                double n2 = is_entering ? mat.refractive_index : 1.0;
                RtVec3 refr_dir;
                if (rt_refract(g_ray.dir, out_n, n1 / n2, refr_dir))
                {
                    g_ray = {v_sub(g_hit.point, v_mul(out_n, EPS)), refr_dir};
                }
                else
                {
                    RtVec3 refl_dir = v_norm(v_sub(g_ray.dir, v_mul(out_n, 2.0 * v_dot(g_ray.dir, out_n))));
                    g_ray = {v_add(g_hit.point, v_mul(out_n, EPS)), refl_dir};
                }
            }
            else
            {
                RtVec3 refl_dir = v_norm(v_sub(g_ray.dir, v_mul(out_n, 2.0 * v_dot(g_ray.dir, out_n))));
                g_ray = {v_add(g_hit.point, v_mul(out_n, EPS)), refl_dir};
            }
        }

        RtColor c = rt_trace_ray(scene, r, rng);
        sum_c = c_add(sum_c, c);
    }

    double inv_s = 1.0 / samples_per_pixel;
    out_raw = c_mul_s(sum_c, inv_s);
    out_norm = (v_sqlen(sum_n) > 1e-6) ? v_norm(sum_n) : RtVec3{0.0, 0.0, 0.0};
    out_alb = c_mul_s(sum_a, inv_s);
}