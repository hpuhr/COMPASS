
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/json.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsondatamapping.h"
        "${CMAKE_CURRENT_LIST_DIR}/jsonobjectparser.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/jsondatamapping.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonobjectparser.cpp"
)


