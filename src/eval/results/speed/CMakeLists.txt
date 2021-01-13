
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/speedsingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/speedjoined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/speedsingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/speedjoined.cpp"
    )


