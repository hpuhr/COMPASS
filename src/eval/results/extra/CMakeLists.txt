
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/utnssingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/utnsjoined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/utnssingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/utnsjoined.cpp"
    )


