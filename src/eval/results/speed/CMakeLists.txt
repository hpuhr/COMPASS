
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/speed.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/speed.cpp"
    )


