
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbconnection.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbresult.h"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnection.h"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnectionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnectioninfowidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnection.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnectionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnectioninfowidget.cpp"
)


