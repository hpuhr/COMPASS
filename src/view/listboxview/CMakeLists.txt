
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/allbuffertablemodel.h"
        "${CMAKE_CURRENT_LIST_DIR}/allbuffertablewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/buffertablemodel.h"
        "${CMAKE_CURRENT_LIST_DIR}/buffertablewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/listboxviewdatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/listboxview.h"
        "${CMAKE_CURRENT_LIST_DIR}/listboxviewconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/listboxviewdatawidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/listboxviewwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/allbuffertablemodel.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/allbuffertablewidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/buffertablemodel.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/buffertablewidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/listboxview.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/listboxviewdatasource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/listboxviewconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/listboxviewdatawidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/listboxviewwidget.cpp"
)
