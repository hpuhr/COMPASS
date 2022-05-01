
include(${CMAKE_CURRENT_LIST_DIR}/variable/CMakeLists.txt)

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
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentlabelgenerator.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentlabelgeneratorwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/labeldefinition.h"
        "${CMAKE_CURRENT_LIST_DIR}/labeldefinitionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/stringrepresentationcombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/selectdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboassociationcollection.h"
        "${CMAKE_CURRENT_LIST_DIR}/target.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbcontent.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentlabelgenerator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentlabelgeneratorwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/labeldefinition.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/labeldefinitionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dboassociationcollection.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/target.cpp"
)


