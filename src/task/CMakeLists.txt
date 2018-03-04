
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/taskmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortask.h"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/taskmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortaskwidget.cpp"
)


