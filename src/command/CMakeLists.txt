
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbcommand.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcommandlist.h"
    PRIVATE
)


