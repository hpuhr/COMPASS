
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbovariable.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariabledefinition.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariablewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableset.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableorderedset.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableorderedsetwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariabledatatypecombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableselectionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableeditdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariablecreatedialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/metadbovariable.h"
        "${CMAKE_CURRENT_LIST_DIR}/metadbovariablewidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbovariable.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariablewidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableset.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableorderedset.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableorderedsetwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableselectionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableeditdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariablecreatedialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/metadbovariable.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/metadbovariablewidget.cpp"
)


