include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsstatusdialog.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsstatusdialog.cpp"
)
