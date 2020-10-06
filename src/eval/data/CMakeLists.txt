
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdata.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatawidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetdata.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetposition.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdata.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatawidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetdata.cpp"
)


