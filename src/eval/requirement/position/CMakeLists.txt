include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/detail.h"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistance.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrossconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrossconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongacross.h"
        "${CMAKE_CURRENT_LIST_DIR}/along.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/across.h"
        "${CMAKE_CURRENT_LIST_DIR}/acrossconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/acrossconfigwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistanceconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistance.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrossconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrossconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongacross.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/along.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/across.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/acrossconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/acrossconfigwidget.cpp"
)


