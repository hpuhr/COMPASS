
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbconnection.h"
        "${CMAKE_CURRENT_LIST_DIR}/mysqlppconnection.h"
        "${CMAKE_CURRENT_LIST_DIR}/mysqlppconnectionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/mysqlppconnectioninfowidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbresult.h"
        "${CMAKE_CURRENT_LIST_DIR}/mysqlserver.h"
        "${CMAKE_CURRENT_LIST_DIR}/mysqlserverwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/sqlitefile.h"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnection.h"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnectionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnectioninfowidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/bufferwriter.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/bufferwriter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mysqlppconnection.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mysqlppconnectionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mysqlppconnectioninfowidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mysqlserverwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnection.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnectionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnectioninfowidget.cpp"
)


