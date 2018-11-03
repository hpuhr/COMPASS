
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/json.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonmapping.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/jsonmapping.cpp"
)


