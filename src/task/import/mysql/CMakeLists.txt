include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttask.h"
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttaskwidget.cpp"
)

