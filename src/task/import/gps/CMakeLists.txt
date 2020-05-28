include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/gpstrailimporttask.h"
        "${CMAKE_CURRENT_LIST_DIR}/gpstrailimporttaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/gpstrailimporttask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/gpstrailimporttaskwidget.cpp"
)

