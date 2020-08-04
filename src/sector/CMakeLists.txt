
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/sector.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/sector.cpp"
)


