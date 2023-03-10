include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
#        "${CMAKE_CURRENT_LIST_DIR}/detail.h"
        "${CMAKE_CURRENT_LIST_DIR}/distanceconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/distanceconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/distance.h"
        "${CMAKE_CURRENT_LIST_DIR}/along.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/across.h"
        "${CMAKE_CURRENT_LIST_DIR}/acrossconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/acrossconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/latency.h"
        "${CMAKE_CURRENT_LIST_DIR}/latencyconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/latencyconfigwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/distanceconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/distanceconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/distance.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/along.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/across.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/acrossconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/acrossconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/latency.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/latencyconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/latencyconfigwidget.cpp"
    )


