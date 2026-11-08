cmake_minimum_required(VERSION 3.28...4.0)

project(
    Moonlight
    VERSION 0.1.0
    DESCRIPTION "Cross-platform rendering and simulation engine"
    LANGUAGES CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CTest)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/MoonlightOptions.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/MoonlightWarnings.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/MoonlightSanitizers.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/MoonlightDependencies.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/MoonlightTesting.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/MoonlightInstall.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/MoonlightModules.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/MoonlightWorkspace.cmake")

if(
    MOONLIGHT_ENABLE_VULKAN
    AND NOT Vulkan_INCLUDE_DIR
    AND EXISTS "/opt/hfs21.0.700/toolkit/include/vulkan/vulkan.h"
)
    set(Vulkan_INCLUDE_DIR
        "/opt/hfs21.0.700/toolkit/include"
        CACHE PATH "Vulkan header directory"
    )
endif()

add_library(moonlight_options INTERFACE)
add_library(Moonlight::options ALIAS moonlight_options)
set_target_properties(moonlight_options PROPERTIES FOLDER "Runtime/Infrastructure")
target_compile_features(moonlight_options INTERFACE cxx_std_20)
moonlight_apply_warnings(moonlight_options)
moonlight_apply_sanitizers(moonlight_options)

add_subdirectory(src/native/core)

moonlight_discover_modules()

if(MOONLIGHT_BUILD_WORKBENCH)
    add_subdirectory(apps/workbench)
endif()

# Expose the non-CMake workspaces as native IDE targets. Each tool remains
# responsible for its own dependency resolution and language-specific build.
moonlight_add_workspace_targets()

# Keep every native target visible to IDEs and provide one first-class target
# that builds the complete multi-language project, including all discovered
# modules and the Rust, managed, and Python workspaces.
add_library(moonlight_modules INTERFACE)
add_library(Moonlight::modules ALIAS moonlight_modules)
set_target_properties(moonlight_modules PROPERTIES FOLDER "Runtime/Modules")

get_property(moonlight_module_targets GLOBAL PROPERTY MOONLIGHT_MODULE_TARGETS)
if(moonlight_module_targets)
    target_link_libraries(moonlight_modules INTERFACE ${moonlight_module_targets})
endif()

add_custom_target(moonlight_all ALL)
add_dependencies(moonlight_all moonlight_core)
if(TARGET moonlight_workbench)
    add_dependencies(moonlight_all moonlight_workbench)
endif()
if(moonlight_module_targets)
    add_dependencies(moonlight_all ${moonlight_module_targets})
endif()
if(TARGET moonlight_core_tests)
    add_dependencies(moonlight_all moonlight_core_tests)
endif()
get_property(moonlight_module_test_targets GLOBAL PROPERTY MOONLIGHT_MODULE_TEST_TARGETS)
if(moonlight_module_test_targets)
    add_dependencies(moonlight_all ${moonlight_module_test_targets})
endif()
if(TARGET moonlight_rust_build)
    add_dependencies(moonlight_all moonlight_rust_build)
endif()
if(TARGET moonlight_managed_build)
    add_dependencies(moonlight_all moonlight_managed_build)
endif()
if(TARGET moonlight_validate_modules)
    add_dependencies(moonlight_all moonlight_validate_modules)
endif()
set_target_properties(moonlight_all PROPERTIES FOLDER "Build")
