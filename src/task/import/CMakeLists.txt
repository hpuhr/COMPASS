include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttask.h"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttask.h"
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttaskwidget.cpp"
)

IF (jASTERIX_FOUND)
	MESSAGE(STATUS "Including ASTERIX Import Task")

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttask.h"
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttaskwidget.cpp"
)
ENDIF()
