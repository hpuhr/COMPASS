include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/speed.h"
        "${CMAKE_CURRENT_LIST_DIR}/speedconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/speedconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/accelerationcorrect.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/speed.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/speedconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/speedconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/accelerationcorrect.cpp"
    )


