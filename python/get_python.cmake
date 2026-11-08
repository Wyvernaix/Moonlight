include_guard(GLOBAL)

set(MOONLIGHT_PYTHON_MIN_VERSION "3.11" CACHE STRING "Minimum Python version")

find_package(Python3 ${MOONLIGHT_PYTHON_MIN_VERSION} COMPONENTS Interpreter REQUIRED)

message(STATUS "Moonlight Python: ${Python3_EXECUTABLE} (${Python3_VERSION})")

function(moonlight_python_run)
    if(NOT ARGN)
        message(FATAL_ERROR "moonlight_python_run requires a script or module")
    endif()
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" ${ARGN}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE result
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Python command failed with exit code ${result}")
    endif()
endfunction()
