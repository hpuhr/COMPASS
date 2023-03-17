
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/sector.h"
        "${CMAKE_CURRENT_LIST_DIR}/sectorlayer.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationsector.h"
        "${CMAKE_CURRENT_LIST_DIR}/airspacesector.h"
        "${CMAKE_CURRENT_LIST_DIR}/airspace.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/sector.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sectorlayer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationsector.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/airspacesector.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/airspace.cpp"
)


