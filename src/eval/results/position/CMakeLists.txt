
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/positionbase.h"
        "${CMAKE_CURRENT_LIST_DIR}/distancesingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/distancejoined.h"
        "${CMAKE_CURRENT_LIST_DIR}/distancermssingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/distancermsjoined.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongsingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongjoined.h"
        "${CMAKE_CURRENT_LIST_DIR}/acrosssingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/acrossjoined.h"
        "${CMAKE_CURRENT_LIST_DIR}/latencysingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/latencyjoined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/positionbase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/distancesingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/distancejoined.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/distancermssingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/distancermsjoined.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongsingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongjoined.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/acrosssingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/acrossjoined.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/latencysingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/latencyjoined.cpp"
    )
