
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/datasourcemanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/datasourcebase.h"
        "${CMAKE_CURRENT_LIST_DIR}/configurationdatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbdatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbdatasourceselectioncombobox.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/datasourcemanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/datasourcebase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/configurationdatasource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbdatasource.cpp"
)


