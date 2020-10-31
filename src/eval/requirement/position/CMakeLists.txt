include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/positionmaxdistanceconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/positionmaxdistanceconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/positionmaxdistance.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/positionmaxdistanceconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/positionmaxdistanceconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/positionmaxdistance.cpp"
)


