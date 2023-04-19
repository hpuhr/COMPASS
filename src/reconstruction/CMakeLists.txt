
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/kalman_defs.h"
        "${CMAKE_CURRENT_LIST_DIR}/kalman.h"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor.h"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_kalman.h"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_umkalman2d.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/kalman.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_kalman.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_umkalman2d.cpp"
)
