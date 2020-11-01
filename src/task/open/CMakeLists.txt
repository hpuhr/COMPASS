
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/databaseopentask.h"
        "${CMAKE_CURRENT_LIST_DIR}/databaseopentaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/databaseopentask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/databaseopentaskwidget.cpp"
)
