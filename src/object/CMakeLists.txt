
include(${CMAKE_CURRENT_LIST_DIR}/variable/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/source/CMakeLists.txt)

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbobject.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectinfowidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectcombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanagerloadwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinition.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinitionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboaddschemametatabledialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/stringrepresentationcombobox.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbobject.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectinfowidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbobjectmanagerloadwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinition.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinitionwidget.cpp"
)


