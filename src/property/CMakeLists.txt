
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/property.h"
        "${CMAKE_CURRENT_LIST_DIR}/propertylist.h"
        "${CMAKE_CURRENT_LIST_DIR}/property_templates.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/property.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/propertylist.cpp"
    )
