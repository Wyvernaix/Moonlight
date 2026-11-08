include_guard(GLOBAL)

include(GNUInstallDirs)

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/data/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/moonlight/data"
    FILES_MATCHING
        PATTERN "*.json"
        PATTERN "*.toml"
        PATTERN "*.yaml"
        PATTERN "*.yml"
)
