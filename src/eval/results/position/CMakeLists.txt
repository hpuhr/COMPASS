
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/maxdistancesingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistancejoined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/maxdistancesingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistancejoined.cpp"
    )


