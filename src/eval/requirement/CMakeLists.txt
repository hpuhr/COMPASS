include("${CMAKE_CURRENT_LIST_DIR}/detection/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementgroup.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirement.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementgroup.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirement.cpp"
)


