
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/positionbase.h"
        "${CMAKE_CURRENT_LIST_DIR}/distance.h"
        "${CMAKE_CURRENT_LIST_DIR}/distancerms.h"
        "${CMAKE_CURRENT_LIST_DIR}/radarrange.h"
        "${CMAKE_CURRENT_LIST_DIR}/radarazimuth.h"
        "${CMAKE_CURRENT_LIST_DIR}/along.h"
        "${CMAKE_CURRENT_LIST_DIR}/across.h"
        "${CMAKE_CURRENT_LIST_DIR}/latency.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/positionbase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/distance.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/distancerms.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radarrange.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radarazimuth.cpp"    
        "${CMAKE_CURRENT_LIST_DIR}/along.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/across.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/latency.cpp"
    )
