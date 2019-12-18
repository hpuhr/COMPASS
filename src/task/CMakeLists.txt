
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/taskmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/taskmanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/taskmanagerlogwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/task.h"
        "${CMAKE_CURRENT_LIST_DIR}/taskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/databaseopentask.h"
        "${CMAKE_CURRENT_LIST_DIR}/databaseopentaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/manageschematask.h"
        "${CMAKE_CURRENT_LIST_DIR}/manageschemataskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortask.h"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttask.h"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttask.h"
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsstatusdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/postprocesstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/postprocesstaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/taskmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskmanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/taskmanagerlogwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/databaseopentask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/databaseopentaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/manageschematask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/manageschemataskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/managedbobjectstaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mysqldbimporttaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsstatusdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/postprocesstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/postprocesstaskwidget.cpp"
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
