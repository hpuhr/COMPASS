
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/grid2d.h"
        "${CMAKE_CURRENT_LIST_DIR}/grid2dlayer.h"

    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/grid2d.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/grid2dlayer.cpp"
)
