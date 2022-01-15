
include(${CMAKE_CURRENT_LIST_DIR}/variable/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/source/CMakeLists.txt)

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbcontent.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentcombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentmanagerloadwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinition.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinitionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/stringrepresentationcombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/selectdbobjectdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboassociationcollection.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbcontent.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentmanagerloadwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinition.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbolabeldefinitionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dboassociationcollection.cpp"
)


