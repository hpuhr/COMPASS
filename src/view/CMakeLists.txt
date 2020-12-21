
include("${CMAKE_CURRENT_LIST_DIR}/listboxview/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/histogramview/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/scatterplotview/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/viewbase/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/points/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/viewmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainer.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainerconfigwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/viewmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewcontainerconfigwidget.cpp"
)


