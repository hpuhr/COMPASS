
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/manageschematask.h"
        "${CMAKE_CURRENT_LIST_DIR}/manageschemataskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/manageschematask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/manageschemataskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstaskwidget.cpp"
)
