include_guard(GLOBAL)

option(
    MOONLIGHT_BUILD_RUST
    "Build the Cargo workspace as part of the complete CMake build"
    ON
)
option(
    MOONLIGHT_BUILD_MANAGED
    "Build the managed solution as part of the complete CMake build"
    ON
)
option(
    MOONLIGHT_VALIDATE_MODULES
    "Run the Python module validation and catalog export during the complete build"
    ON
)

function(moonlight_add_workspace_targets)
    if(MOONLIGHT_BUILD_RUST)
        find_program(MOONLIGHT_CARGO_EXECUTABLE cargo REQUIRED)

        add_custom_target(moonlight_rust_build
            COMMAND
                "${MOONLIGHT_CARGO_EXECUTABLE}" build --workspace --locked
                --profile "$<IF:$<CONFIG:Release>,release,dev>"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            USES_TERMINAL
            VERBATIM
        )
        set_target_properties(moonlight_rust_build PROPERTIES FOLDER "Build/Rust")

        # Cargo owns compilation. The small launcher is a real CMake
        # executable so CLion can build, run, and debug the Cargo binary.
        add_executable(moonlight_module_graph
            "${CMAKE_SOURCE_DIR}/tools/module-graph/launcher.cpp"
        )
        target_compile_definitions(moonlight_module_graph PRIVATE
            MOONLIGHT_MODULE_GRAPH_PATH="$<IF:$<CONFIG:Release>,${CMAKE_SOURCE_DIR}/target/release/module-graph,${CMAKE_SOURCE_DIR}/target/debug/module-graph>"
        )
        set_target_properties(moonlight_module_graph PROPERTIES
            FOLDER "Applications/Rust"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
        add_dependencies(moonlight_module_graph moonlight_rust_build)

        add_custom_target(moonlight_rust_check
            COMMAND
                "${MOONLIGHT_CARGO_EXECUTABLE}" fmt --all -- --check
            COMMAND
                "${MOONLIGHT_CARGO_EXECUTABLE}" clippy --workspace --all-targets
                --locked -- -D warnings
            COMMAND
                "${MOONLIGHT_CARGO_EXECUTABLE}" test --workspace --locked
                --profile "$<IF:$<CONFIG:Release>,release,dev>"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            USES_TERMINAL
            VERBATIM
        )
        set_target_properties(moonlight_rust_check PROPERTIES FOLDER "Tests/Rust")
    endif()

    if(MOONLIGHT_BUILD_MANAGED)
        find_program(MOONLIGHT_DOTNET_EXECUTABLE dotnet REQUIRED)

        add_custom_target(moonlight_managed_restore
            COMMAND
                "${MOONLIGHT_DOTNET_EXECUTABLE}" restore
                "${CMAKE_SOURCE_DIR}/Moonlight.sln"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            USES_TERMINAL
            VERBATIM
        )
        set_target_properties(
            moonlight_managed_restore
            PROPERTIES FOLDER "Build/Managed"
        )

        add_custom_target(moonlight_managed_build
            COMMAND
                "${MOONLIGHT_DOTNET_EXECUTABLE}" build
                "${CMAKE_SOURCE_DIR}/Moonlight.sln"
                --no-restore
                --configuration "$<CONFIG>"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            DEPENDS moonlight_managed_restore
            USES_TERMINAL
            VERBATIM
        )
        set_target_properties(
            moonlight_managed_build
            PROPERTIES FOLDER "Build/Managed"
        )

        add_custom_target(moonlight_managed_check
            COMMAND
                "${MOONLIGHT_DOTNET_EXECUTABLE}" test
                "${CMAKE_SOURCE_DIR}/Moonlight.sln"
                --configuration "$<CONFIG>"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            DEPENDS moonlight_managed_build
            USES_TERMINAL
            VERBATIM
        )
        set_target_properties(
            moonlight_managed_check
            PROPERTIES FOLDER "Tests/Managed"
        )
    endif()

    if(MOONLIGHT_VALIDATE_MODULES)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        set(module_catalog "${CMAKE_BINARY_DIR}/graph/module-catalog.json")

        add_custom_target(moonlight_validate_modules
            COMMAND
                "${CMAKE_COMMAND}" -E make_directory
                "${CMAKE_BINARY_DIR}/graph"
            COMMAND
                "${Python3_EXECUTABLE}" scripts/moonlight.py validate
            COMMAND
                "${Python3_EXECUTABLE}" scripts/moonlight.py export
                --out "${module_catalog}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            BYPRODUCTS "${module_catalog}"
            USES_TERMINAL
            VERBATIM
        )
        set_target_properties(
            moonlight_validate_modules
            PROPERTIES FOLDER "Build/Python"
        )
    endif()

    add_custom_target(moonlight_workspace_check)
    if(TARGET moonlight_rust_check)
        add_dependencies(moonlight_workspace_check moonlight_rust_check)
    endif()
    if(TARGET moonlight_managed_check)
        add_dependencies(moonlight_workspace_check moonlight_managed_check)
    endif()
    if(TARGET moonlight_validate_modules)
        add_dependencies(moonlight_workspace_check moonlight_validate_modules)
    endif()
    set_target_properties(
        moonlight_workspace_check
        PROPERTIES FOLDER "Tests"
    )
endfunction()
