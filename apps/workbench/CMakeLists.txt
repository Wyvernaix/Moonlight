include(FetchContent)

find_package(SDL3 CONFIG REQUIRED)

# Some Moonlight development images provide Vulkan through the HFS toolkit
# rather than the distribution include path. Prefer normal CMake discovery,
# then use that SDK automatically so the Workbench preset needs no local
# profile options.
if(NOT Vulkan_INCLUDE_DIR AND EXISTS "/opt/hfs21.0.700/toolkit/include/vulkan/vulkan.h")
    set(Vulkan_INCLUDE_DIR
        "/opt/hfs21.0.700/toolkit/include"
        CACHE PATH "Vulkan header directory"
    )
endif()
find_package(Vulkan REQUIRED)

cmake_policy(SET CMP0169 OLD)
FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9b.tar.gz
    URL_HASH SHA256=8e1bbc76c71d74fef2fb85db7e7ca8eba13d6a86623c54992b60162db554ffdb
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

add_library(moonlight_imgui STATIC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
)
target_include_directories(moonlight_imgui PUBLIC
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends"
)
target_link_libraries(moonlight_imgui PUBLIC SDL3::SDL3 Vulkan::Vulkan)
set_target_properties(moonlight_imgui PROPERTIES FOLDER "Applications/Editor")

add_executable(moonlight_workbench
    src/main.cpp
)
target_link_libraries(moonlight_workbench PRIVATE
    Moonlight::core
    Moonlight::module_atlas
    Moonlight::module_spectral
    Moonlight::options
    moonlight_imgui
    SDL3::SDL3
    Vulkan::Vulkan
)
target_include_directories(moonlight_workbench PRIVATE "${imgui_SOURCE_DIR}")
set_target_properties(moonlight_workbench PROPERTIES
    FOLDER "Applications/Editor"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
