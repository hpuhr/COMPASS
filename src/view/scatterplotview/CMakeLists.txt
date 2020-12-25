
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotviewdatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotview.h"
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotviewconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotviewchartview.h"
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotviewdatawidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotviewwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotview.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotviewdatasource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotviewconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotviewchartview.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotviewdatawidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/scatterplotviewwidget.cpp"
)


