include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/get_python.cmake")

if(UNIX AND NOT APPLE)
    include("${CMAKE_CURRENT_LIST_DIR}/Platform/Linux/install_files_linux.cmake")
elseif(APPLE)
    include("${CMAKE_CURRENT_LIST_DIR}/Platform/Mac/install_files_mac.cmake")
elseif(WIN32)
    include("${CMAKE_CURRENT_LIST_DIR}/Platform/Windows/install_files_windows.cmake")
endif()