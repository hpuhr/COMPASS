include("${CMAKE_CURRENT_LIST_DIR}/report/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/detection/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/position/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/identification/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/mode_a/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/mode_c/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/extra/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationresultsgenerator.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationresultsgeneratorwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/base.h"
        "${CMAKE_CURRENT_LIST_DIR}/single.h"
        "${CMAKE_CURRENT_LIST_DIR}/joined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationresultsgenerator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationresultsgeneratorwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/base.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/single.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/joined.cpp"
    )


