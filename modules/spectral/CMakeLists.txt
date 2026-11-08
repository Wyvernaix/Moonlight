add_library(moonlight_module_spectral STATIC src/spectral.cpp)
add_library(Moonlight::module_spectral ALIAS moonlight_module_spectral)
target_include_directories(moonlight_module_spectral PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(moonlight_module_spectral PUBLIC Moonlight::options)
target_link_libraries(moonlight_module_spectral PUBLIC Moonlight::module_atlas)

if(MOONLIGHT_ENABLE_VULKAN)
    find_package(Vulkan REQUIRED)
    find_program(
        MOONLIGHT_GLSLC_EXECUTABLE
        glslc
        HINTS
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/shaderc"
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin"
    )
    if(NOT MOONLIGHT_GLSLC_EXECUTABLE)
        message(
            FATAL_ERROR
            "Moonlight requires glslc to compile Spectral shaders. "
            "Run 'python python/bootstrap.py' or install the Vulkan SDK."
        )
    endif()

    set(spectral_shader_dir "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    set(spectral_shader_output_dir "${CMAKE_CURRENT_BINARY_DIR}/shaders")
    add_custom_command(
        OUTPUT
            "${spectral_shader_output_dir}/sphere.vert.spv"
            "${spectral_shader_output_dir}/sphere.frag.spv"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${spectral_shader_output_dir}"
        COMMAND "${MOONLIGHT_GLSLC_EXECUTABLE}"
            "${spectral_shader_dir}/sphere.vert"
            -o "${spectral_shader_output_dir}/sphere.vert.spv"
        COMMAND "${MOONLIGHT_GLSLC_EXECUTABLE}"
            "${spectral_shader_dir}/sphere.frag"
            -o "${spectral_shader_output_dir}/sphere.frag.spv"
        DEPENDS
            "${spectral_shader_dir}/sphere.vert"
            "${spectral_shader_dir}/sphere.frag"
        VERBATIM
    )
    add_custom_target(moonlight_spectral_shaders
        DEPENDS
            "${spectral_shader_output_dir}/sphere.vert.spv"
            "${spectral_shader_output_dir}/sphere.frag.spv"
    )
    target_sources(moonlight_module_spectral PRIVATE src/vulkan_renderer.cpp)
    target_link_libraries(moonlight_module_spectral PUBLIC Vulkan::Vulkan)
    target_compile_definitions(moonlight_module_spectral PRIVATE
        MOONLIGHT_SPECTRAL_VERT="${spectral_shader_output_dir}/sphere.vert.spv"
        MOONLIGHT_SPECTRAL_FRAG="${spectral_shader_output_dir}/sphere.frag.spv"
    )
    add_dependencies(moonlight_module_spectral moonlight_spectral_shaders)
endif()

if(MOONLIGHT_BUILD_TESTS)
    add_executable(moonlight_module_spectral_tests tests/spectral_tests.cpp)
    target_link_libraries(moonlight_module_spectral_tests PRIVATE Moonlight::module_spectral)
    moonlight_register_test(moonlight_module_spectral_tests)
endif()
