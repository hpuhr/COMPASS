include("${CMAKE_CURRENT_LIST_DIR}/data/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/requirement/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/standard/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagermaintabwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatasourcewidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagermaintabwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatasourcewidget.cpp"
)


