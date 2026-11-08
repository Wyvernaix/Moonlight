include_guard(GLOBAL)

function(moonlight_register_test target)
    if(NOT MOONLIGHT_BUILD_TESTS)
        return()
    endif()

    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(
        ${target}
        PROPERTIES
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            ENVIRONMENT "MOONLIGHT_DATA_ROOT=${CMAKE_SOURCE_DIR}/data"
    )
endfunction()
