
include("${CMAKE_CURRENT_LIST_DIR}/listboxview/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/viewbase/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/viewmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainer.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainerconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewpoint.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/viewmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainerconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewpoint.cpp"
)


