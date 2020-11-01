
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/sector.h"
        "${CMAKE_CURRENT_LIST_DIR}/sectorlayer.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/sector.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sectorlayer.cpp"
)


