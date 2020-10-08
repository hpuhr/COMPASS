include("${CMAKE_CURRENT_LIST_DIR}/report/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/detection/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/position/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationresultsgenerator.h"
        "${CMAKE_CURRENT_LIST_DIR}/base.h"
        "${CMAKE_CURRENT_LIST_DIR}/single.h"
        "${CMAKE_CURRENT_LIST_DIR}/joined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationresultsgenerator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/base.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/single.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/joined.cpp"
    )


