
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbobject.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectinfowidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectcombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariablewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanagerloadwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariabledatatypecombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableselectionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariabledatatypecombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinitionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboadddatasourcedialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariable.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableset.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableorderedset.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableorderedsetwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinition.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinitionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboaddschemametatabledialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/metadbovariable.h"
        "${CMAKE_CURRENT_LIST_DIR}/metadbovariablewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/stringrepresentationcombobox.h"
    #        src/object/dbovariablecombobox.h
    #        src/object/dbovariablesetwidget.h
    #        src/object/propertydatatypecombobox.h
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbobject.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectinfowidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanagerloadwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinitionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariable.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariablewidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableset.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableorderedset.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableorderedsetwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbovariableselectionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinition.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinitionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/metadbovariable.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/metadbovariablewidget.cpp"
    #        src/object/dbovariablecombobox.cpp
    #        src/object/dbovariablesetwidget.cpp
)


