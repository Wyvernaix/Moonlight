cmake_minimum_required(VERSION 3.28...4.0)

project(
    Moonlight
    VERSION 0.1.0
    DESCRIPTION "Cross-platform rendering and simulation engine"
    LANGUAGES CXX
)

include(CTest)

option(MOONLIGHT_BUILD_TESTS "Build Moonlight tests" ON)
option(MOONLIGHT_ENABLE_WARNINGS "Enable strict compiler warnings" ON)

add_library(moonlight_options INTERFACE)
add_library(Moonlight::options ALIAS moonlight_options)

target_compile_features(moonlight_options INTERFACE cxx_std_23)

if(MOONLIGHT_ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(moonlight_options INTERFACE /W4 /permissive-)
    else()
        target_compile_options(
            moonlight_options
            INTERFACE
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wsign-conversion
        )
    endif()
endif()

if(MOONLIGHT_BUILD_TESTS)
    enable_testing()
endif()