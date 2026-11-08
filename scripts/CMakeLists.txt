include(GNUInstallDirs)

install(
    PROGRAMS
        "${CMAKE_CURRENT_LIST_DIR}/moonlight.sh"
        "${CMAKE_CURRENT_LIST_DIR}/moonlight.py"
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT tools
)