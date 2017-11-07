
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/property.h"
        "${CMAKE_CURRENT_LIST_DIR}/propertylist.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/property.cpp"
    )


