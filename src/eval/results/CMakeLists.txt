include("${CMAKE_CURRENT_LIST_DIR}/report/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationresultsgenerator.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementresult.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementdetectionresult.h"
        "${CMAKE_CURRENT_LIST_DIR}/singleevaluationrequirementdetectionresult.h"
        "${CMAKE_CURRENT_LIST_DIR}/joinedevaluationrequirementdetectionresult.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationresultsgenerator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementresult.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementdetectionresult.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/singleevaluationrequirementdetectionresult.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/joinedevaluationrequirementdetectionresult.cpp"
    )


