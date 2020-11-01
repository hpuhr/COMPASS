
include("${CMAKE_CURRENT_LIST_DIR}/assoc/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/calc/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/import/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/manage/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/open/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/source/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/taskmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/taskmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/taskmanagerlogwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/task.h"
        "${CMAKE_CURRENT_LIST_DIR}/taskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/taskmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskmanagerlogwidget.cpp"
)
