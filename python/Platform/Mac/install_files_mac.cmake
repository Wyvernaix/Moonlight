install(
    DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../../"
    DESTINATION "share/moonlight/python"
    COMPONENT tools
    FILES_MATCHING
    PATTERN "*.py"
    PATTERN "*.txt"
    PATTERN "*.cmake"
    PATTERN "*.plist"
)