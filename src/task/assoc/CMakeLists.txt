include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsstatusdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationstaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationsstatusdialog.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsstatusdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationstaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationsstatusdialog.cpp"
)
