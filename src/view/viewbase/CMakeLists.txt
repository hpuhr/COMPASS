
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/view.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewcomponent.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewtoolwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewdatawidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewtoolswitcher.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewloadstatewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewpresetwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewpresets.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewvariable.h"
        "${CMAKE_CURRENT_LIST_DIR}/variableview.h"
        "${CMAKE_CURRENT_LIST_DIR}/variableviewconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/variableviewdatawidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/chartview.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewscreenshotdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/colormap.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewevaldata.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewevaldatawidget.h"
        
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/view.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewcomponent.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewtoolwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewdatawidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewtoolswitcher.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewloadstatewidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewpresetwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewpresets.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewvariable.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/variableview.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/variableviewconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/variableviewdatawidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/chartview.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewscreenshotdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/colormap.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewevaldata.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewevaldatawidget.cpp"
)
