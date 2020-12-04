include("${CMAKE_CURRENT_LIST_DIR}/detection/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/position/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/identification/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/mode_a/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/mode_c/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/extra/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/config.h"
        "${CMAKE_CURRENT_LIST_DIR}/group.h"
        "${CMAKE_CURRENT_LIST_DIR}/base.h"
        "${CMAKE_CURRENT_LIST_DIR}/checkdetail.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/config.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/group.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/base.cpp"
)


