
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/trackangle.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/trackangle.cpp"
    )
