
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationstandard.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationstandardwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationstandardcombobox.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationstandard.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationstandardwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationstandardcombobox.cpp"
)


