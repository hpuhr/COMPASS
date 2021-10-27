
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/activedatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinition.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinitionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourceselectioncombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentdatasourcebase.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentconfigurationdatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentdbdatasource.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinition.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinitionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentdatasourcebase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentconfigurationdatasource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentdbdatasource.cpp"
)


