include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/speed.h"
        "${CMAKE_CURRENT_LIST_DIR}/speedconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/speedconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/detail.h"        
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/speed.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/speedconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/speedconfigwidget.cpp"
    )


