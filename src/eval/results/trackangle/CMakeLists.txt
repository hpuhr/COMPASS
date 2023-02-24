
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/trackanglesingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/trackanglejoined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/trackanglesingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trackanglejoined.cpp"
    )


