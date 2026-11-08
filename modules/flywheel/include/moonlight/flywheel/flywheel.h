#pragma once

#include <cstdint>

namespace moonlight::flywheel {

struct ModuleInfo {
    std::uint32_t api_version;
    const char* name;
};

[[nodiscard]] ModuleInfo module_info() noexcept;

}  // namespace moonlight::flywheel
