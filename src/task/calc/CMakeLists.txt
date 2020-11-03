
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortask.h"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/postprocesstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/postprocesstaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/postprocesstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/postprocesstaskwidget.cpp"
)
