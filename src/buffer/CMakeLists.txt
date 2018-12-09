
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/nullablevector.h"
        "${CMAKE_CURRENT_LIST_DIR}/buffer.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/nullablevector.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/buffer.cpp"
)


