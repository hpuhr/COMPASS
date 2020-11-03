
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/invalidqlineedit.h"
        "${CMAKE_CURRENT_LIST_DIR}/textfielddoublevalidator.h"
        "${CMAKE_CURRENT_LIST_DIR}/textfieldhexvalidator.h"
        "${CMAKE_CURRENT_LIST_DIR}/textfieldoctvalidator.h"
    PRIVATE
)


