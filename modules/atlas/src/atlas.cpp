#include <moonlight/atlas/atlas.h>

#include <utility>

namespace moonlight::atlas {

Scene::Scene(std::string name) : name_(std::move(name)) {}

std::uint32_t Scene::create_entity(
    std::string name,
    Vec3 position,
    Geometry geometry,
    float geometry_radius
) {
    const auto id = next_entity_id_++;
    entities_.push_back(Entity{
        .id = id,
        .name = std::move(name),
        .transform = Transform{.position = position},
        .geometry = geometry,
        .geometry_radius = geometry_radius,
    });
    return id;
}

const std::string& Scene::name() const noexcept {
    return name_;
}

const std::vector<Entity>& Scene::entities() const noexcept {
    return entities_;
}

Scene create_default_scene() {
    Scene scene{"Workbench Scene"};
    static_cast<void>(scene.create_entity(
        "Origin", {.x = 0.0F, .y = 0.0F, .z = 0.0F},
        Geometry::Sphere, 1.0F
    ));
    static_cast<void>(scene.create_entity(
        "Atlas Node", {.x = 2.0F, .y = 0.0F, .z = 1.0F}
    ));
    static_cast<void>(scene.create_entity(
        "Spectral Node", {.x = -2.0F, .y = 0.0F, .z = -1.0F}
    ));
    return scene;
}

ModuleInfo module_info() noexcept {
    return {.api_version = 1U, .name = "atlas"};
}

}  // namespace moonlight::atlas
