include_guard(GLOBAL)

function(moonlight_discover_modules)
    file(GLOB module_manifests CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/modules/*/module.json"
    )
    foreach(manifest IN LISTS module_manifests)
        get_filename_component(module_directory "${manifest}" DIRECTORY)
        get_filename_component(module_name "${module_directory}" NAME)
        file(READ "${manifest}" manifest_json)
        string(JSON manifest_schema_version ERROR_VARIABLE manifest_json_error GET
            "${manifest_json}" schemaVersion
        )
        if(manifest_json_error)
            message(FATAL_ERROR
                "Module '${module_name}' has invalid JSON in ${manifest}: "
                "${manifest_json_error}"
            )
        endif()
        string(JSON manifest_id GET "${manifest_json}" id)
        string(JSON manifest_target GET "${manifest_json}" cmakeTarget)
        if(NOT manifest_id STREQUAL module_name)
            message(FATAL_ERROR
                "Module manifest directory '${module_name}' declares id "
                "'${manifest_id}'."
            )
        endif()
        if(manifest_target STREQUAL "")
            message(FATAL_ERROR
                "Module '${module_name}' must declare a non-empty cmakeTarget."
            )
        endif()
        if(NOT EXISTS "${module_directory}/CMakeLists.txt")
            message(FATAL_ERROR
                "Module '${module_name}' is missing ${module_directory}/CMakeLists.txt."
            )
        endif()

        add_subdirectory(
            "${module_directory}"
            "${CMAKE_BINARY_DIR}/modules/${module_name}"
        )
        if(TARGET "${manifest_target}")
            set_target_properties(
                "${manifest_target}"
                PROPERTIES FOLDER "Modules/${module_name}"
            )
            set_property(
                GLOBAL APPEND PROPERTY MOONLIGHT_MODULE_TARGETS
                "${manifest_target}"
            )
        else()
            message(FATAL_ERROR
                "Module '${module_name}' declares CMake target "
                "'${manifest_target}', but that target was not created."
            )
        endif()
        if(TARGET "moonlight_module_${module_name}_tests")
            set_target_properties(
                "moonlight_module_${module_name}_tests"
                PROPERTIES FOLDER "Tests/Modules/${module_name}"
            )
            set_property(
                GLOBAL APPEND PROPERTY MOONLIGHT_MODULE_TEST_TARGETS
                "moonlight_module_${module_name}_tests"
            )
        endif()
    endforeach()
endfunction()
