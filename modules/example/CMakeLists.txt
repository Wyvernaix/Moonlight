add_library(moonlight_module_example STATIC
    src/example.cpp
)
add_library(Moonlight::module_example ALIAS moonlight_module_example)
target_include_directories(moonlight_module_example
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(moonlight_module_example PUBLIC Moonlight::options)

if(MOONLIGHT_BUILD_TESTS)
    add_executable(moonlight_module_example_tests tests/example_tests.cpp)
    target_link_libraries(moonlight_module_example_tests PRIVATE Moonlight::module_example)
    add_test(NAME moonlight_module_example_tests COMMAND moonlight_module_example_tests)
endif()
