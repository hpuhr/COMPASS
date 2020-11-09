
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/targetreport.h"
        "${CMAKE_CURRENT_LIST_DIR}/target.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/targetreport.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target.cpp"
)


