include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/complexaccuracyestimator.h"
        "${CMAKE_CURRENT_LIST_DIR}/datasourceaccuracyestimator.h"
        "${CMAKE_CURRENT_LIST_DIR}/adsbaccuracyestimator.h"
        "${CMAKE_CURRENT_LIST_DIR}/mlataccuracyestimator.h"
        "${CMAKE_CURRENT_LIST_DIR}/trackeraccuracyestimator.h"
        "${CMAKE_CURRENT_LIST_DIR}/radaraccuracyestimator.h"
        "${CMAKE_CURRENT_LIST_DIR}/probimmreconstructor.h"
        "${CMAKE_CURRENT_LIST_DIR}/probimmreconstructorwidget.h"

    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/complexaccuracyestimator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/datasourceaccuracyestimator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/adsbaccuracyestimator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mlataccuracyestimator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/trackeraccuracyestimator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radaraccuracyestimator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/probimmreconstructor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/probimmreconstructorwidget.cpp"
)
