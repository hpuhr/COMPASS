include("${CMAKE_CURRENT_LIST_DIR}/data/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/requirement/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/results/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/standard/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmaintabwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetstabwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationstandardtabwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationresultstabwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatasourcewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationsectorwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmaintabwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetstabwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationstandardtabwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationresultstabwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatasourcewidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationsectorwidget.cpp"
)


