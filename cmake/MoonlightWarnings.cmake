include_guard(GLOBAL)

function(moonlight_apply_warnings target)
    if(NOT MOONLIGHT_ENABLE_WARNINGS)
        return()
    endif()

    get_target_property(target_type "${target}" TYPE)
    if(target_type STREQUAL "INTERFACE_LIBRARY")
        set(scope INTERFACE)
    else()
        set(scope PRIVATE)
    endif()

    if(MSVC)
        target_compile_options(${target} ${scope} /W4 /permissive- /w14242 /w14254)
    else()
        target_compile_options(
            ${target}
            ${scope}
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wsign-conversion
                -Wshadow
                -Werror=return-type
        )
    endif()
endfunction()
