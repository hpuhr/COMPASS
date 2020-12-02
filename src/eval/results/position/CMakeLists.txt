
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/maxdistancesingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistancejoined.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrosssingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrossjoined.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongsingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/alongjoined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/maxdistancesingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/maxdistancejoined.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrosssingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongacrossjoined.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongsingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/alongjoined.cpp"
    )


