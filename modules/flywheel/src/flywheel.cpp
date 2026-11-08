#include <moonlight/flywheel/flywheel.h>

namespace moonlight::flywheel {

ModuleInfo module_info() noexcept {
    return {.api_version = 1U, .name = "flywheel"};
}

}  // namespace moonlight::flywheel
