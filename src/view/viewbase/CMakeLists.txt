
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/view.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/view.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewwidget.cpp"
)


