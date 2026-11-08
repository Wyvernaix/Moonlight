#include <moonlight/atlas/atlas.h>

#include <cassert>

#include <cassert>

int main() {
    const auto info = moonlight::atlas::module_info();
    assert(info.api_version == 1U);
    assert(info.name != nullptr);
    const auto scene = moonlight::atlas::create_default_scene();
    assert(scene.entities().size() == 3U);
    assert(scene.entities().front().id == 1U);
    assert(scene.entities().front().geometry == moonlight::atlas::Geometry::Sphere);
    return 0;
}
