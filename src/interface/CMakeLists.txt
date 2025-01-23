
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbtableinfo.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbinterface.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbresult.h"
        "${CMAKE_CURRENT_LIST_DIR}/sqlgenerator.h"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnection.h"
        "${CMAKE_CURRENT_LIST_DIR}/duckdbconnection.h"
        "${CMAKE_CURRENT_LIST_DIR}/duckdbscopedappender.h"
        "${CMAKE_CURRENT_LIST_DIR}/duckdbscopedprepare.h"
        "${CMAKE_CURRENT_LIST_DIR}/duckdbconnectionsettings.h"
        "${CMAKE_CURRENT_LIST_DIR}/duckdbcommon.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbinterface.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sqlgenerator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sqliteconnection.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/duckdbconnection.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/duckdbscopedappender.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/duckdbscopedprepare.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/duckdbconnectionsettings.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/duckdbcommon.cpp"
)
