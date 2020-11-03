
include("${CMAKE_CURRENT_LIST_DIR}/rs2g/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/ogr/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/projection.h"
        #"${CMAKE_CURRENT_LIST_DIR}/geomap.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/projection.cpp"
        #"${CMAKE_CURRENT_LIST_DIR}/geomap.cpp"
)


