#include <iostream>
#include <fstream>
#include <memory>

#include <Maths/Vector.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Floor.hpp>
#include <Scene/Environment.hpp>
#include <Objects/Ball.hpp>
#include <Objects/Attributes/Material.hpp>
#include <Color/Absorption.hpp>
#include <Objects/Attributes/Textures/MarbleTexture.hpp>
#include <Objects/Attributes/Textures/CheckerTexture.hpp>

// ============================================================================
// Scène vitrine : montre l'ensemble des fonctionnalités du moteur
//
//   Rangée du fond   : verres (incolore, saphir, émeraude, ambre)
//                      → réfraction, Fresnel, absorption de Beer-Lambert
//   Rangée du milieu : miroir net, miroir rugueux, marbre en relief, damier
//                      → réflexions nettes/floues, Perlin, bump mapping, UV
//   Rangée de devant : mat, mat brillant, diamant
//                      → diffus, Fresnel sur surface rugueuse, indice élevé
//   Sol              : damier avec atténuation selon la distance
//   Éclairage        : une lumière sphérique (ombres douces)
//                      + une lumière ponctuelle (ombres dures)
//   Ciel             : dégradé de coucher de soleil avec soleil visible
// ============================================================================

static void addBall(Scene& scene, const Vector3D& pos, double radius, const std::shared_ptr<Material>& mat) {
    scene.addObject(std::make_shared<Ball>(pos, radius, mat));
}

int main() {
    // ── Résolution ────────────────────────────────────────────────────────
    const uint16_t image_width = 1920;
    const uint16_t image_height = 1080;
    const short samples_per_pixel = 128;
    const double aspect_ratio = static_cast<double>(image_width) / image_height;

    // ── Caméra ────────────────────────────────────────────────────────────
    // Viewport placé plus loin (1.8) pour un champ de vision plus serré :
    // moins de déformation des sphères sur les bords de l'image.
    // Il est décalé vers le bas (-0.6) pour que la caméra regarde légèrement vers le sol.
    const double viewport_height = 2.0;
    const double viewport_width = viewport_height * aspect_ratio;
    const double viewport_distance = 1.8;

    Vector3D pov(0.0, 3.0, -6.0);
    Vector3D viewport_center = pov + Vector3D(0.0, -0.6, viewport_distance);
    Vector3D vech(viewport_width, 0.0, 0.0);
    Vector3D vecy(0.0, viewport_height, 0.0);
    Viewport viewport(viewport_center - vech / 2.0 - vecy / 2.0, vech, vecy);

    // ── Environnement : coucher de soleil, soleil bas et visible devant ───
    Environment sunset(
        Color{255, 150, 85},                                            // horizon
        Color{40, 60, 125},                                             // zénith
        Sun(Vector3D(-0.35, 0.08, 1.0), Color{255, 235, 200}, 1.2)
    );

    Scene scene(pov, viewport, sunset, 6);

    // ── Sol : damier atténué avec la distance (plan y = -1) ──────────────
    scene.addObject(std::make_shared<Floor>(Vector3D(), -1.0));

    // ── Rangée du fond (z = 8) : verres et absorption volumique ──────────
    const double back_z = 8.0;
    addBall(scene, Vector3D(-7.5, 0.0, back_z), 1.0, Material::Glass(1.5, Absorption::None()));     // verre clair
    addBall(scene, Vector3D(-2.5, 0.0, back_z), 1.0, Material::Glass(1.5, Absorption::Saphire()));  // saphir
    addBall(scene, Vector3D( 2.5, 0.0, back_z), 1.0, Material::Glass(1.5, Absorption::Emerald()));  // émeraude
    addBall(scene, Vector3D( 7.5, 0.0, back_z), 1.0, Material::Glass(1.5, Absorption::Amber()));    // ambre

    // ── Rangée du milieu (z = 3) : réflexions et textures ────────────────
    const double mid_z = 3.0;
    const double mid_r = 0.75;
    const double mid_y = -1.0 + mid_r;

    // Miroir parfaitement net
    addBall(scene, Vector3D(-3.6, mid_y, mid_z), mid_r, Material::Mirror(Color{235, 235, 240}, 0.95));

    // Miroir doré rugueux : réflexions floues
    addBall(scene, Vector3D(-1.2, mid_y, mid_z), mid_r, Material::Mirror(Color{255, 200, 110}, 0.85, 0.25));

    // Marbre procédural (Perlin + turbulence) avec bump mapping
    auto marble = std::make_shared<MarbleTexture>(Color{245, 240, 235}, Color{35, 45, 55}, 2.5, 6.0);
    addBall(scene, Vector3D(1.2, mid_y, mid_z), mid_r, Material::Matte(marble, 0.08, 0.02));

    // Damier plaqué sur une sphère (coordonnées UV sphériques)
    auto sphere_checker = std::make_shared<CheckerTexture>(Color{230, 60, 60}, Color{250, 245, 235}, 12.0, false);
    addBall(scene, Vector3D(3.6, mid_y, mid_z), mid_r, Material::Matte(sphere_checker));

    // ── Rangée de devant (z = 0) : matériaux simples ─────────────────────
    const double front_z = 0.0;
    const double front_r = 0.4;
    const double front_y = -1.0 + front_r;

    // Mat pur (diffus lambertien)
    addBall(scene, Vector3D(-1.6, front_y, front_z), front_r, Material::Matte(Color{60, 140, 255}));

    // Mat rugueux : reflets de Fresnel en bord de sphère
    addBall(scene, Vector3D(0.0, front_y, front_z), front_r, Material::Matte(Color{255, 40, 40}, 0.4));

    // Diamant : indice de réfraction très élevé
    addBall(scene, Vector3D(1.6, front_y, front_z), front_r, Material::Glass(2.42));

    // ── Lumières ──────────────────────────────────────────────────────────
    // Principale : sphérique (rayon 0.8) → pénombre douce
    scene.addLightSource(std::make_shared<LightSource>(Vector3D(-3.0, 6.0, -1.0), 0.9, 0.8));
    // Secondaire : ponctuelle (rayon 0) → ombres dures, plus faible
    scene.addLightSource(std::make_shared<LightSource>(Vector3D(5.0, 4.0, 1.0), 0.35, 0.0));

    // ── Rendu ─────────────────────────────────────────────────────────────
    #ifdef USE_GPU
        Frame frame = scene.renderGPU(image_width, image_height, samples_per_pixel);
    #else
        Frame frame = scene.render(image_width, image_height, samples_per_pixel);
    #endif

    // ── Export PPM ────────────────────────────────────────────────────────
    std::ofstream file("output.ppm");
    if (!file.is_open()) {
        std::cerr << "Erreur lors de l'ouverture du fichier !" << std::endl;
        return 1;
    }

    file << "P3" << std::endl << image_width << ' ' << image_height << std::endl << "255" << std::endl;

    for (uint32_t i = 0; i < static_cast<uint32_t>(image_width) * image_height; i++) {
        const Color& c = frame.getPixels()[i];
        file << static_cast<int>(c.r) << ' ' << static_cast<int>(c.g) << ' ' << static_cast<int>(c.b) << '\n';
    }

    file.close();
    std::cout << "Image generee avec succes dans 'output.ppm' !" << std::endl;
    return 0;
}
