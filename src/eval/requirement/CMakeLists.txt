include("${CMAKE_CURRENT_LIST_DIR}/detection/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/config.h"
        "${CMAKE_CURRENT_LIST_DIR}/group.h"
        "${CMAKE_CURRENT_LIST_DIR}/base.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/config.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/group.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/base.cpp"
)


