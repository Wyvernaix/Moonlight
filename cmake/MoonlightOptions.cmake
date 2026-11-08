include_guard(GLOBAL)

option(MOONLIGHT_BUILD_TESTS "Build Moonlight tests" ON)
option(MOONLIGHT_ENABLE_WARNINGS "Enable strict compiler warnings" ON)
option(MOONLIGHT_ENABLE_SANITIZERS "Enable supported native sanitizers" OFF)
option(MOONLIGHT_USE_VCPKG_DEPENDENCIES "Require optional vcpkg dependencies" OFF)
option(MOONLIGHT_ENABLE_GRAPHICS "Build graphics-facing modules" OFF)
option(MOONLIGHT_ENABLE_VULKAN "Build the Vulkan Spectral backend" OFF)
option(MOONLIGHT_ENABLE_SHADERS "Build shader tooling integration" OFF)
option(
    MOONLIGHT_BUILD_WORKBENCH
    "Build the native Dear ImGui Workbench application"
    OFF
)

# Keep the former cache variable working for users with an existing CLion
# profile, while making Workbench the canonical name.
if(MOONLIGHT_BUILD_EDITOR)
    set(MOONLIGHT_BUILD_WORKBENCH ON CACHE BOOL
        "Build the native Dear ImGui Workbench application" FORCE
    )
endif()
