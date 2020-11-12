include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/modeaconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/modeaconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/modea.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/modeaconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/modeaconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/modea.cpp"
)


