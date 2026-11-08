#include <moonlight/flywheel/flywheel.h>

#include <cassert>

int main() {
    const auto info = moonlight::flywheel::module_info();
    assert(info.api_version == 1U);
    assert(info.name != nullptr);
    return 0;
}
