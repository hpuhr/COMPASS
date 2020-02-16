
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/projection.h"
        "${CMAKE_CURRENT_LIST_DIR}/rs2gprojection.h"
        "${CMAKE_CURRENT_LIST_DIR}/rs2gcoordinatesystem.h"
        #"${CMAKE_CURRENT_LIST_DIR}/geomap.h"
        "${CMAKE_CURRENT_LIST_DIR}/rs2g.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/projection.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rs2gprojection.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rs2gcoordinatesystem.cpp"
        #"${CMAKE_CURRENT_LIST_DIR}/geomap.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rs2g.cpp"

)


