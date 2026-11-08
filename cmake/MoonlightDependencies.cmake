include_guard(GLOBAL)

add_library(moonlight_dependencies INTERFACE)
add_library(Moonlight::dependencies ALIAS moonlight_dependencies)

if(NOT MOONLIGHT_USE_VCPKG_DEPENDENCIES)
    return()
endif()

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)

target_link_libraries(
    moonlight_dependencies
    INTERFACE
        fmt::fmt
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        yaml-cpp::yaml-cpp
        glm::glm
        SDL3::SDL3
        Vulkan::Headers
)

if(MOONLIGHT_ENABLE_SHADERS)
    find_package(shaderc CONFIG REQUIRED)
    target_link_libraries(moonlight_dependencies INTERFACE shaderc::shaderc)
endif()
