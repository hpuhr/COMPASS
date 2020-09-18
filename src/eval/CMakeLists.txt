
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatasourcewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdata.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetdata.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatasourcewidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdata.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetdata.cpp"
)


