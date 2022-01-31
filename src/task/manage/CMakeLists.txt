
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/managesectorstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/managesectorstaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/importsectordialog.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/managesectorstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/managesectorstaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/importsectordialog.cpp"
    )
