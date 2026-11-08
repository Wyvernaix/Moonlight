#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace moonlight::atlas {

struct Vec3 {
    float x{};
    float y{};
    float z{};
};

struct Transform {
    Vec3 position{};
};

enum class Geometry : std::uint8_t {
    None,
    Sphere,
};

struct Entity {
    std::uint32_t id{};
    std::string name;
    Transform transform{};
    Geometry geometry{Geometry::None};
    float geometry_radius{1.0F};
};

class Scene {
public:
    explicit Scene(std::string name = "Untitled Scene");

    [[nodiscard]] std::uint32_t create_entity(
        std::string name,
        Vec3 position,
        Geometry geometry = Geometry::None,
        float geometry_radius = 1.0F
    );
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] const std::vector<Entity>& entities() const noexcept;

private:
    std::string name_;
    std::vector<Entity> entities_;
    std::uint32_t next_entity_id_{1U};
};

[[nodiscard]] Scene create_default_scene();

struct ModuleInfo {
    std::uint32_t api_version;
    const char* name;
};

[[nodiscard]] ModuleInfo module_info() noexcept;

}  // namespace moonlight::atlas
