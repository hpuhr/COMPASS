
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/datasingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/datajoined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/datasingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/datajoined.cpp"
    )


