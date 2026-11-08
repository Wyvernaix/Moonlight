include_guard(GLOBAL)

function(moonlight_discover_modules)
    file(GLOB module_manifests CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/modules/*/module.json"
    )
    foreach(manifest IN LISTS module_manifests)
        get_filename_component(module_directory "${manifest}" DIRECTORY)
        get_filename_component(module_name "${module_directory}" NAME)
        if(EXISTS "${module_directory}/CMakeLists.txt")
            add_subdirectory(
                "${module_directory}"
                "${CMAKE_BINARY_DIR}/modules/${module_name}"
            )
        endif()
    endforeach()
endfunction()
