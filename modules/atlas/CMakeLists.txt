add_library(moonlight_module_atlas STATIC src/atlas.cpp)
add_library(Moonlight::module_atlas ALIAS moonlight_module_atlas)
target_include_directories(moonlight_module_atlas PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(moonlight_module_atlas PUBLIC Moonlight::options)

if(MOONLIGHT_BUILD_TESTS)
    add_executable(moonlight_module_atlas_tests tests/atlas_tests.cpp)
    target_link_libraries(moonlight_module_atlas_tests PRIVATE Moonlight::module_atlas)
    moonlight_register_test(moonlight_module_atlas_tests)
endif()
