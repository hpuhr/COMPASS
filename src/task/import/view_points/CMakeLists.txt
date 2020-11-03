include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/viewpointsimporttask.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewpointsimporttaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/viewpointsimporttask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewpointsimporttaskwidget.cpp"
)


