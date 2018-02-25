
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/geomap.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/projectionmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/geomap.cpp"

)


