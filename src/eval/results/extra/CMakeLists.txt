
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/datasingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/datajoined.h"
        "${CMAKE_CURRENT_LIST_DIR}/tracksingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/trackjoined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/datasingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/datajoined.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/tracksingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trackjoined.cpp"
    )


