
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbtable.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbtablecolumn.h"
        "${CMAKE_CURRENT_LIST_DIR}/metadbtable.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbtablecolumncombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbschema.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbschemawidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbschemamanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbschemamanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbtablewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/metadbtablewidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbschemawidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbtablewidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbtablecolumncombobox.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbtable.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbtablecolumn.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbschema.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbschemamanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbschemamanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/metadbtable.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/metadbtablewidget.cpp"
)


