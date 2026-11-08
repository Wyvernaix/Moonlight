#pragma once

#include <cstdint>
#include <vector>

#include "moonlight/atlas/atlas.h"

namespace moonlight::spectral {

struct Vec2 {
    float x{};
    float y{};
};

struct Color {
    std::uint32_t rgba{};
};

struct ViewportCamera {
    float yaw{0.8F};
    float pitch{0.65F};
    float distance{8.0F};
    atlas::Vec3 target{};
};

struct GridLine {
    Vec2 start;
    Vec2 end;
    Color color;
    float thickness{1.0F};
};

struct EntityMarker {
    Vec2 position;
    Color color;
    float radius{6.0F};
    std::uint32_t entity_id{};
};

struct SceneView {
    std::vector<GridLine> grid;
    std::vector<EntityMarker> entities;
    Vec2 origin{};
};

[[nodiscard]] SceneView build_scene_view(
    const atlas::Scene& scene,
    const ViewportCamera& camera,
    float width,
    float height
);

struct ModuleInfo {
    std::uint32_t api_version;
    const char* name;
};

[[nodiscard]] ModuleInfo module_info() noexcept;

}  // namespace moonlight::spectral
