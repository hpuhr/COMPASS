
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinition.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinitionwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboadddatasourcedialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceswidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceaction.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceactionoptions.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceactionoptionswidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/storeddbodatasource.h"
        "${CMAKE_CURRENT_LIST_DIR}/storeddbodatasourcewidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcewidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinition.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbodatasourcedefinitionwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceswidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceaction.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceactionoptions.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceactionoptionswidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/storeddbodatasource.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/storeddbodatasourcewidget.cpp"
)


