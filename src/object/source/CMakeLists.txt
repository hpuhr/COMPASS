
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinitionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboadddatasourcedialog.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinitionwidget.cpp"
)


