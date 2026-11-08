add_library(moonlight_core STATIC
    src/runtime.cpp
)
add_library(Moonlight::core ALIAS moonlight_core)
set_target_properties(moonlight_core PROPERTIES FOLDER "Runtime/Core")

target_include_directories(
    moonlight_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(moonlight_core PUBLIC Moonlight::options Moonlight::dependencies)

if(MOONLIGHT_BUILD_TESTS)
    add_executable(moonlight_core_tests tests/runtime_tests.cpp)
    target_link_libraries(moonlight_core_tests PRIVATE Moonlight::core)
    set_target_properties(moonlight_core_tests PROPERTIES FOLDER "Tests/Runtime")
    moonlight_register_test(moonlight_core_tests)
endif()
