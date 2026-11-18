#define RT_DEVICE_ONLY
#include "RenderCore.hpp"
#include "RenderCore.cpp"
#undef RT_DEVICE_ONLY

#include <hip/hip_runtime.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

#define HIP_CHECK(cmd)                                                                                    \
    do                                                                                                    \
    {                                                                                                     \
        hipError_t err = (cmd);                                                                           \
        if (err != hipSuccess)                                                                            \
        {                                                                                                 \
            std::fprintf(stderr, "Erreur HIP (%s:%d): %s\n", __FILE__, __LINE__, hipGetErrorString(err)); \
            std::exit(1);                                                                                 \
        }                                                                                                 \
    } while (0)

__global__ void render_kernel(
    uint16_t image_width,
    uint16_t image_height,
    short samples_per_pixel,
    RtSceneData scene,
    RtColor *out_raw,
    RtVec3 *out_norm,
    RtColor *out_alb)
{
    uint16_t x = blockIdx.x * blockDim.x + threadIdx.x;
    uint16_t y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= image_width || y >= image_height)
        return;

    size_t idx = static_cast<size_t>(y) * image_width + x;
    rt_render_pixel(x, y, image_width, image_height, samples_per_pixel, scene, out_raw[idx], out_norm[idx], out_alb[idx]);
}

void launch_hip_render(
    uint16_t image_width,
    uint16_t image_height,
    short samples_per_pixel,
    const RtSceneData &host_scene,
    RtColor *out_raw,
    RtVec3 *out_norm,
    RtColor *out_alb)
{
    int device_count = 0;
    HIP_CHECK(hipGetDeviceCount(&device_count));
    if (device_count == 0)
    {
        std::fprintf(stderr, "Aucun GPU AMD compatible HIP detecte !\n");
        std::exit(1);
    }

    size_t total_pixels = static_cast<size_t>(image_width) * image_height;

    RtSphere *d_spheres = nullptr;
    RtPlane *d_planes = nullptr;
    RtMaterial *d_materials = nullptr;
    RtLight *d_lights = nullptr;
    RtColor *d_raw = nullptr;
    RtVec3 *d_norm = nullptr;
    RtColor *d_alb = nullptr;
    int *d_perm = nullptr;

    size_t sz_spheres = std::max(1, host_scene.num_spheres) * sizeof(RtSphere);
    size_t sz_planes = std::max(1, host_scene.num_planes) * sizeof(RtPlane);
    size_t sz_mats = std::max(1, host_scene.num_materials) * sizeof(RtMaterial);
    size_t sz_lights = std::max(1, host_scene.num_lights) * sizeof(RtLight);
    size_t sz_perm = 512 * sizeof(int);

    HIP_CHECK(hipMalloc(&d_spheres, sz_spheres));
    HIP_CHECK(hipMalloc(&d_planes, sz_planes));
    HIP_CHECK(hipMalloc(&d_materials, sz_mats));
    HIP_CHECK(hipMalloc(&d_lights, sz_lights));
    HIP_CHECK(hipMalloc(&d_perm, sz_perm));
    HIP_CHECK(hipMalloc(&d_raw, total_pixels * sizeof(RtColor)));
    HIP_CHECK(hipMalloc(&d_norm, total_pixels * sizeof(RtVec3)));
    HIP_CHECK(hipMalloc(&d_alb, total_pixels * sizeof(RtColor)));

    if (host_scene.num_spheres > 0)
        HIP_CHECK(hipMemcpy(d_spheres, host_scene.spheres, sz_spheres, hipMemcpyHostToDevice));
    if (host_scene.num_planes > 0)
        HIP_CHECK(hipMemcpy(d_planes, host_scene.planes, sz_planes, hipMemcpyHostToDevice));
    if (host_scene.num_materials > 0)
        HIP_CHECK(hipMemcpy(d_materials, host_scene.materials, sz_mats, hipMemcpyHostToDevice));
    if (host_scene.num_lights > 0)
        HIP_CHECK(hipMemcpy(d_lights, host_scene.lights, sz_lights, hipMemcpyHostToDevice));

    if (host_scene.perlin_perm != nullptr)
        HIP_CHECK(hipMemcpy(d_perm, host_scene.perlin_perm, sz_perm, hipMemcpyHostToDevice));

    RtSceneData dev_scene = host_scene;
    dev_scene.spheres = d_spheres;
    dev_scene.planes = d_planes;
    dev_scene.materials = d_materials;
    dev_scene.lights = d_lights;
    dev_scene.perlin_perm = d_perm;

    dim3 block_size(16, 16);
    dim3 grid_size(
        (image_width + block_size.x - 1) / block_size.x,
        (image_height + block_size.y - 1) / block_size.y);

    HIP_CHECK(hipDeviceSetLimit(hipLimitStackSize, 16384));

    hipLaunchKernelGGL(
        render_kernel,
        grid_size, block_size, 0, 0,
        image_width, image_height, samples_per_pixel,
        dev_scene, d_raw, d_norm, d_alb);

    HIP_CHECK(hipGetLastError());
    HIP_CHECK(hipDeviceSynchronize());

    HIP_CHECK(hipMemcpy(out_raw, d_raw, total_pixels * sizeof(RtColor), hipMemcpyDeviceToHost));
    HIP_CHECK(hipMemcpy(out_norm, d_norm, total_pixels * sizeof(RtVec3), hipMemcpyDeviceToHost));
    HIP_CHECK(hipMemcpy(out_alb, d_alb, total_pixels * sizeof(RtColor), hipMemcpyDeviceToHost));

    HIP_CHECK(hipFree(d_spheres));
    HIP_CHECK(hipFree(d_planes));
    HIP_CHECK(hipFree(d_materials));
    HIP_CHECK(hipFree(d_lights));
    HIP_CHECK(hipFree(d_raw));
    HIP_CHECK(hipFree(d_norm));
    HIP_CHECK(hipFree(d_alb));
    HIP_CHECK(hipFree(d_perm));
}