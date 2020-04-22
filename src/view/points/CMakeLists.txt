include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/viewpoint.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewpointswidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewpointswidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewpointstoolwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewpointstablemodel.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/viewpoint.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewpointswidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewpointstoolwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewpointstablemodel.cpp"
)


