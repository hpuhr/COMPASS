
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/taskmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortask.h"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimportertask.h"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimportertaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/taskmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/radarplotpositioncalculatortaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimportertask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimportertaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskwidget.cpp"
)

IF (jASTERIX_FOUND)
	MESSAGE(STATUS "Including ASTERIX Import Task")

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/asteriximportertask.h"
        "${CMAKE_CURRENT_LIST_DIR}/asteriximportertaskwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/asteriximportertask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/asteriximportertaskwidget.cpp"
)
ENDIF()
