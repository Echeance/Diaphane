#include "Scene.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <thread>

#include <Objects/Attributes/Shapes/Sphere.hpp>
#include <Objects/Attributes/Shapes/Plane.hpp>
#include <Objects/Attributes/Textures/CheckerTexture.hpp>
#include <Objects/Attributes/Textures/MarbleTexture.hpp>

Scene::Scene(const Vector3D& _pov, const Viewport& _viewport, const Environment& _environment, short _reflection_depth) :
    pov(_pov),
    viewport(_viewport),
    environment(_environment),
    reflection_depth(_reflection_depth)
{}

void Scene::addLightSource(const std::shared_ptr<LightSource>& ls) {
    this->light_sources.push_back(ls);
}

void Scene::addObject(const std::shared_ptr<Object>& obj) {
    this->objects.push_back(obj);
}

Scene::PackedScene Scene::pack_scene() const {
    PackedScene packed;

    for (const auto& obj : this->objects) {
        const auto& mat = obj->getMaterial();
        RtMaterial rm{};
        rm.reflectivity = mat->getReflectivity();
        rm.transparency = mat->getTransparency();
        rm.refractive_index = mat->getRefractiveIndex();
        rm.specular_intensity = mat->getSpecularIntensity();
        rm.shininess = mat->getShininess();
        rm.roughness = mat->getRoughness();
        rm.bump_strength = mat->getBumpStrength();
        rm.absorption = {
            mat->getAbsorption()->getRed(),
            mat->getAbsorption()->getGreen(),
            mat->getAbsorption()->getBlue()
        };

        Color c0 = obj->getColor(obj->getCenter());
        rm.base_color = {c0.r, c0.g, c0.b};
        rm.texture_type = 0;

        if (auto checker = std::dynamic_pointer_cast<CheckerTexture>(mat->getTexture())) {
            rm.texture_type = 1;
            Color ce = checker->getEvenColor();
            Color co = checker->getOddColor();
            rm.base_color = {ce.r, ce.g, ce.b};
            rm.secondary_color = {co.r, co.g, co.b};
            rm.tex_scale = checker->getScale();
            rm.tex_distance_fade = checker->hasDistanceFade();
        } else if (auto marble = std::dynamic_pointer_cast<MarbleTexture>(mat->getTexture())) {
            rm.texture_type = 2;
            const Color& c1 = marble->getColor1();
            const Color& c2 = marble->getColor2();
            rm.base_color = {c1.r, c1.g, c1.b};
            rm.secondary_color = {c2.r, c2.g, c2.b};
            rm.tex_scale = marble->getScale();
            rm.tex_turbulence = marble->getTurbulenceStrength();
            packed.perlin_perm = marble->getPerm();
            packed.data.perlin_perm = packed.perlin_perm.data();
        }

        int mat_idx = static_cast<int>(packed.materials.size());
        packed.materials.push_back(rm);

        if (auto sphere = std::dynamic_pointer_cast<Sphere>(obj->getShape())) {
            packed.spheres.push_back({
                {obj->getCenter().getX(), obj->getCenter().getY(), obj->getCenter().getZ()},
                sphere->getRadius(),
                mat_idx
            });
        } else if (auto plane = std::dynamic_pointer_cast<Plane>(obj->getShape())) {
            const Vector3D& n = plane->getNormal();
            packed.planes.push_back({
                {n.getX(), n.getY(), n.getZ()},
                plane->getD(),
                mat_idx
            });
        }
    }

    for (const auto& ls : this->light_sources) {
        packed.lights.push_back({
            {ls->pos.getX(), ls->pos.getY(), ls->pos.getZ()},
            ls->intensity,
            ls->radius
        });
    }

    const Sun& sun = this->environment.getSun();
    packed.data = RtSceneData{
        packed.spheres.data(), static_cast<int>(packed.spheres.size()),
        packed.planes.data(), static_cast<int>(packed.planes.size()),
        packed.materials.data(), static_cast<int>(packed.materials.size()),
        packed.lights.data(), static_cast<int>(packed.lights.size()),
        RtCamera{
            {pov.getX(), pov.getY(), pov.getZ()},
            {viewport.origin.getX(), viewport.origin.getY(), viewport.origin.getZ()},
            {viewport.vech.getX(), viewport.vech.getY(), viewport.vech.getZ()},
            {viewport.vecy.getX(), viewport.vecy.getY(), viewport.vecy.getZ()}
        },
        RtEnvironment{
            {environment.getHorizon().r, environment.getHorizon().g, environment.getHorizon().b},
            {environment.getZenith().r, environment.getZenith().g, environment.getZenith().b},
            sun.enabled,
            {sun.direction.getX(), sun.direction.getY(), sun.direction.getZ()},
            {sun.color.r, sun.color.g, sun.color.b},
            sun.intensity
        },
        this->reflection_depth,
        packed.perlin_perm.data()
    };

    return packed;
}

void Scene::denoise(
    const std::vector<RtColor>& raw_color,
    const std::vector<RtVec3>& normals,
    const std::vector<RtColor>& albedos,
    Frame& out_frame,
    uint16_t width,
    uint16_t height,
    double exposure
) const {
    constexpr int radius = 3;
    constexpr double sigma_spatial = 2.0;
    constexpr double sigma_normal = 0.35;
    constexpr double sigma_albedo = 25.0;

    for (uint16_t y = 0; y < height; ++y) {
        for (uint16_t x = 0; x < width; ++x) {
            size_t center_idx = static_cast<size_t>(y) * width + x;
            const RtColor& c_center = raw_color[center_idx];
            const RtVec3& n_center = normals[center_idx];
            const RtColor& a_center = albedos[center_idx];

            // Si le rayon (direct ou réfléchi) atteint le ciel, on ne floute pas
            if (v_sqlen(n_center) < 1e-4) {
                out_frame.getPixel(x, y) = Color(c_center.r, c_center.g, c_center.b).toneMap(exposure);
                continue;
            }

            RtColor sum_color{0.0, 0.0, 0.0};
            double sum_weight = 0.0;

            for (int dy = -radius; dy <= radius; ++dy) {
                int ny = static_cast<int>(y) + dy;
                if (ny < 0 || ny >= height) continue;

                for (int dx = -radius; dx <= radius; ++dx) {
                    int nx = static_cast<int>(x) + dx;
                    if (nx < 0 || nx >= width) continue;

                    size_t idx = static_cast<size_t>(ny) * width + nx;
                    const RtVec3& n_sample = normals[idx];
                    if (v_sqlen(n_sample) < 1e-4) continue;

                    double dist_sq = dx * dx + dy * dy;
                    double w_spatial = std::exp(-dist_sq / (2.0 * sigma_spatial * sigma_spatial));

                    double norm_diff = v_sqlen(v_sub(n_center, n_sample));
                    double w_normal = std::exp(-norm_diff / (2.0 * sigma_normal * sigma_normal));

                    const RtColor& a_sample = albedos[idx];
                    double dr = a_center.r - a_sample.r;
                    double dg = a_center.g - a_sample.g;
                    double db = a_center.b - a_sample.b;
                    double w_albedo = std::exp(-(dr * dr + dg * dg + db * db) / (2.0 * sigma_albedo * sigma_albedo));

                    double weight = w_spatial * w_normal * w_albedo;
                    sum_color = c_add(sum_color, c_mul_s(raw_color[idx], weight));
                    sum_weight += weight;
                }
            }

            RtColor denoised = (sum_weight > 0.0) ? c_mul_s(sum_color, 1.0 / sum_weight) : c_center;
            out_frame.getPixel(x, y) = Color(denoised.r, denoised.g, denoised.b).toneMap(exposure);
        }
    }
}

Frame Scene::render(uint16_t image_width, uint16_t image_height, short samples_per_pixel) const {
    PackedScene packed = this->pack_scene();
    packed.data.perlin_perm = packed.perlin_perm.data();

    size_t total_pixels = static_cast<size_t>(image_width) * image_height;
    std::vector<RtColor> raw_colors(total_pixels);
    std::vector<RtVec3> g_normals(total_pixels);
    std::vector<RtColor> g_albedos(total_pixels);

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    std::atomic<uint16_t> next_y{0};
    std::vector<std::thread> workers;
    workers.reserve(num_threads);

    auto render_worker = [&]() {
        while (true) {
            uint16_t y = next_y.fetch_add(1);
            if (y >= image_height) break;

            for (uint16_t x = 0; x < image_width; ++x) {
                size_t idx = static_cast<size_t>(y) * image_width + x;
                rt_render_pixel(x, y, image_width, image_height, samples_per_pixel, packed.data, raw_colors[idx], g_normals[idx], g_albedos[idx]);
            }
        }
    };

    for (unsigned int i = 0; i < num_threads; ++i)
        workers.emplace_back(render_worker);

    for (auto& worker : workers)
        worker.join();

    Frame frame(image_width, image_height);
    this->denoise(raw_colors, g_normals, g_albedos, frame, image_width, image_height, 0.75);
    return frame;
}

#ifdef USE_GPU
Frame Scene::renderGPU(uint16_t image_width, uint16_t image_height, short samples_per_pixel) const {
    PackedScene packed = this->pack_scene();
    packed.data.perlin_perm = packed.perlin_perm.data();

    size_t total_pixels = static_cast<size_t>(image_width) * image_height;
    std::vector<RtColor> raw_colors(total_pixels);
    std::vector<RtVec3> g_normals(total_pixels);
    std::vector<RtColor> g_albedos(total_pixels);

    launch_hip_render(
        image_width, image_height, samples_per_pixel,
        packed.data,
        raw_colors.data(), g_normals.data(), g_albedos.data()
    );

    Frame frame(image_width, image_height);
    this->denoise(raw_colors, g_normals, g_albedos, frame, image_width, image_height, 0.75);
    return frame;
}
#endif