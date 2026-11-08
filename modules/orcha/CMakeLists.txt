add_library(moonlight_module_orcha STATIC src/orcha.cpp)
add_library(Moonlight::module_orcha ALIAS moonlight_module_orcha)
target_include_directories(moonlight_module_orcha PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(moonlight_module_orcha PUBLIC Moonlight::options)

if(MOONLIGHT_BUILD_TESTS)
    add_executable(moonlight_module_orcha_tests tests/orcha_tests.cpp)
    target_link_libraries(moonlight_module_orcha_tests PRIVATE Moonlight::module_orcha)
    moonlight_register_test(moonlight_module_orcha_tests)
endif()
