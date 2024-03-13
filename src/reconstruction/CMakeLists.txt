
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/kalman_defs.h"
        "${CMAKE_CURRENT_LIST_DIR}/kalman.h"
        "${CMAKE_CURRENT_LIST_DIR}/kalman_projection.h"
        "${CMAKE_CURRENT_LIST_DIR}/kalman_interface.h"
        "${CMAKE_CURRENT_LIST_DIR}/kalman_interface_linear.h"
        "${CMAKE_CURRENT_LIST_DIR}/kalman_interface_umkalman2d.h"
        "${CMAKE_CURRENT_LIST_DIR}/kalman_estimator.h"

        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_defs.h"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor.h"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_kalman.h"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_kalman_projection.h"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_umkalman2d.h"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_interp.h"
        "${CMAKE_CURRENT_LIST_DIR}/spline_interpolator.h"
    
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/kalman.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/kalman_projection.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/kalman_interface.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/kalman_interface_linear.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/kalman_interface_umkalman2d.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/kalman_estimator.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/reconstructor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_kalman.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_kalman_projection.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_umkalman2d.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructor_interp.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/spline_interpolator.cpp"
        
)
