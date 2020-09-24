include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementdetectionconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementdetectionconfigwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementdetectionconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationrequirementdetectionconfigwidget.cpp"
)


