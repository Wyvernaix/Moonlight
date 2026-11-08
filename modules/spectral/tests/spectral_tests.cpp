#include <moonlight/spectral/spectral.h>

#include <cassert>

#include <cassert>

int main() {
    const auto info = moonlight::spectral::module_info();
    assert(info.api_version == 1U);
    assert(info.name != nullptr);
    const auto scene = moonlight::atlas::create_default_scene();
    const auto view = moonlight::spectral::build_scene_view(
        scene, {}, 800.0F, 600.0F
    );
    assert(view.grid.size() == 42U);
    assert(view.entities.size() == scene.entities().size());
    return 0;
}
