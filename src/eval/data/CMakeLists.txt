
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdata.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatawidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatafilterdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetdata.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetposition.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetvelocity.h"
        "${CMAKE_CURRENT_LIST_DIR}/timeperiod.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdata.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatawidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationdatafilterdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationtargetdata.cpp"
)


