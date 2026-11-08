#pragma once

#include <cstdint>

namespace moonlight::orcha {

struct ModuleInfo {
    std::uint32_t api_version;
    const char* name;
};

[[nodiscard]] ModuleInfo module_info() noexcept;

}  // namespace moonlight::orcha
