
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/histogramviewdatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/histogramview.h"
        "${CMAKE_CURRENT_LIST_DIR}/histogramviewconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/histogramviewdatawidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/histogramviewwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/histogramviewchartview.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/histogramview.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/histogramviewdatasource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/histogramviewconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/histogramviewdatawidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/histogramviewwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/histogramviewchartview.cpp"
)


