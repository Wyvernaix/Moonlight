#include <moonlight/spectral/spectral.h>

#include <algorithm>
#include <cmath>

namespace moonlight::spectral {

namespace {

Vec2 project(atlas::Vec3 point, const ViewportCamera& camera, float width, float height) {
    const float cos_yaw = std::cos(camera.yaw);
    const float sin_yaw = std::sin(camera.yaw);
    const float horizontal_x = point.x - camera.target.x;
    const float horizontal_z = point.z - camera.target.z;
    const float rotated_x = horizontal_x * cos_yaw - horizontal_z * sin_yaw;
    const float rotated_z = horizontal_x * sin_yaw + horizontal_z * cos_yaw;
    const float cos_pitch = std::cos(camera.pitch);
    const float sin_pitch = std::sin(camera.pitch);
    const float view_y = (point.y - camera.target.y) * cos_pitch - rotated_z * sin_pitch;
    const float view_z = (point.y - camera.target.y) * sin_pitch + rotated_z * cos_pitch;
    const float scale = std::min(width, height) * 0.12F / std::max(camera.distance, 0.1F);
    return {
        .x = width * 0.5F + rotated_x * scale,
        .y = height * 0.5F - view_y * scale - view_z * scale * 0.18F,
    };
}

} // namespace

SceneView build_scene_view(
    const atlas::Scene& scene,
    const ViewportCamera& camera,
    float width,
    float height
) {
    SceneView view;
    view.origin = project(camera.target, camera, width, height);
    constexpr int grid_extent = 10;
    for (int i = -grid_extent; i <= grid_extent; ++i) {
        const auto x_start = project(
            {.x = static_cast<float>(i), .y = 0.0F, .z = -grid_extent},
            camera, width, height
        );
        const auto x_end = project(
            {.x = static_cast<float>(i), .y = 0.0F, .z = grid_extent},
            camera, width, height
        );
        const auto z_start = project(
            {.x = -grid_extent, .y = 0.0F, .z = static_cast<float>(i)},
            camera, width, height
        );
        const auto z_end = project(
            {.x = grid_extent, .y = 0.0F, .z = static_cast<float>(i)},
            camera, width, height
        );
        const auto color = i == 0 ? 0x506070FFU : 0x293442FFU;
        view.grid.push_back({x_start, x_end, {color}, i == 0 ? 2.0F : 1.0F});
        view.grid.push_back({z_start, z_end, {color}, i == 0 ? 2.0F : 1.0F});
    }
    for (const auto& entity : scene.entities()) {
        const auto position = project(entity.transform.position, camera, width, height);
        const auto color = entity.id == 1U ? 0xE8B86AFF : 0x68A9E8FF;
        view.entities.push_back({position, {color}, 7.0F, entity.id});
    }
    return view;
}

ModuleInfo module_info() noexcept {
    return {.api_version = 1U, .name = "spectral"};
}

}  // namespace moonlight::spectral
