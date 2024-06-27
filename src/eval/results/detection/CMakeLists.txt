
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/detection.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/detection.cpp"
    )
