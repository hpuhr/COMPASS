
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/correctsingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/correctjoined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/correctsingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/correctjoined.cpp"
    )


