include("${CMAKE_CURRENT_LIST_DIR}/requirement/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatasourcewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdata.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatawidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetdata.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationstandard.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatasourcewidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdata.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatawidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetdata.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationstandard.cpp"
)


