include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttask.h"
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttaskwidget.cpp"
)

