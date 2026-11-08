add_library(moonlight_module_flywheel STATIC src/flywheel.cpp)
add_library(Moonlight::module_flywheel ALIAS moonlight_module_flywheel)
target_include_directories(moonlight_module_flywheel PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(moonlight_module_flywheel PUBLIC Moonlight::options)

if(MOONLIGHT_BUILD_TESTS)
    add_executable(moonlight_module_flywheel_tests tests/flywheel_tests.cpp)
    target_link_libraries(moonlight_module_flywheel_tests PRIVATE Moonlight::module_flywheel)
    moonlight_register_test(moonlight_module_flywheel_tests)
endif()
