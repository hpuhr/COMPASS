
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/genericsingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/genericjoined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/genericsingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/genericjoined.cpp"
    )


