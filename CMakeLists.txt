cmake_minimum_required(VERSION 3.28...4.0)

project(
    Moonlight
    VERSION 0.1.0
    DESCRIPTION "Cross-platform rendering and simulation engine"
    LANGUAGES CXX
)

include(CTest)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/MoonlightModules.cmake")

option(MOONLIGHT_BUILD_TESTS "Build Moonlight tests" ON)
option(MOONLIGHT_ENABLE_WARNINGS "Enable strict compiler warnings" ON)

add_library(moonlight_options INTERFACE)
add_library(Moonlight::options ALIAS moonlight_options)
target_compile_features(moonlight_options INTERFACE cxx_std_20)

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

add_library(moonlight_core STATIC
    src/native/core/src/runtime.cpp
)
add_library(Moonlight::core ALIAS moonlight_core)
target_include_directories(
    moonlight_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/native/core/include>
)
target_link_libraries(moonlight_core PUBLIC Moonlight::options)

if(MOONLIGHT_BUILD_TESTS)
    add_executable(moonlight_core_tests
        src/native/core/tests/runtime_tests.cpp
    )
    target_link_libraries(moonlight_core_tests PRIVATE Moonlight::core)
    add_test(NAME moonlight_core_tests COMMAND moonlight_core_tests)
endif()

moonlight_discover_modules()
