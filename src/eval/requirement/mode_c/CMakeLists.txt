include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/modecconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/modecconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/modec.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/modecconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/modecconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/modec.cpp"
)


