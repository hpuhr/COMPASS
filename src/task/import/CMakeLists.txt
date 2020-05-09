
IF (jASTERIX_FOUND)
include("${CMAKE_CURRENT_LIST_DIR}/asterix/CMakeLists.txt")
ENDIF()

include("${CMAKE_CURRENT_LIST_DIR}/gps/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/json/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/mysql/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/view_points/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )
