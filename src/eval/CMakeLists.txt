
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagerwidget.h"
        #"${CMAKE_CURRENT_LIST_DIR}/dbinterfaceinfowidget.h"
        #"${CMAKE_CURRENT_LIST_DIR}/sqlgenerator.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/evaluationmanagerwidget.cpp"
        #"${CMAKE_CURRENT_LIST_DIR}/sqlgenerator.cpp"
)


