
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/autoresumedialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/invalidqlineedit.h"
        "${CMAKE_CURRENT_LIST_DIR}/textfielddoublevalidator.h"
        "${CMAKE_CURRENT_LIST_DIR}/textfieldhexvalidator.h"
        "${CMAKE_CURRENT_LIST_DIR}/textfieldoctvalidator.h"
        "${CMAKE_CURRENT_LIST_DIR}/rangeedit.h"
        "${CMAKE_CURRENT_LIST_DIR}/rangeslider.h"
        "${CMAKE_CURRENT_LIST_DIR}/rangeslider_p.h"
        "${CMAKE_CURRENT_LIST_DIR}/formatselectionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/datatypeformatselectionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/groupbox.h"
        "${CMAKE_CURRENT_LIST_DIR}/selectdatasourceswidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/autoresumedialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rangeedit.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rangeslider.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/formatselectionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/datatypeformatselectionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/groupbox.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/selectdatasourceswidget.cpp"
)
