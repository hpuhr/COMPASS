
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/managedatasourcestask.h"
        "${CMAKE_CURRENT_LIST_DIR}/managedatasourcestaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/managedatasourcestask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/managedatasourcestaskwidget.cpp"
)
