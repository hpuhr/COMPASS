include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistance.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrossconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrossconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongacross.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistance.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrossconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrossconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongacross.cpp"
)


