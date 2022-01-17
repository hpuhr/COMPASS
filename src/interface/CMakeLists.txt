
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbtableinfo.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbinterface.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbinterfaceinfowidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbresult.h"
        "${CMAKE_CURRENT_LIST_DIR}/sqlgenerator.h"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnection.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbinterface.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbinterfaceinfowidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sqlgenerator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnection.cpp"
)


