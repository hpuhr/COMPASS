include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistance.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistance.cpp"
)


