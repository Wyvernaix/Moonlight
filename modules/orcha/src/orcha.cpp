#include <moonlight/orcha/orcha.h>

namespace moonlight::orcha {

ModuleInfo module_info() noexcept {
    return {.api_version = 1U, .name = "orcha"};
}

}  // namespace moonlight::orcha
