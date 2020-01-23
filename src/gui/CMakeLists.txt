
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/invalidqlineedit.h"
        "${CMAKE_CURRENT_LIST_DIR}/textfielddoublevalidator.h"
    PRIVATE
)


