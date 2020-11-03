
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/managedatasourcestask.h"
        "${CMAKE_CURRENT_LIST_DIR}/managedatasourcestaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceswidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceaction.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceactionoptions.h"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceactionoptionswidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/storeddbodatasource.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/managedatasourcestask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/managedatasourcestaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceswidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceaction.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceactionoptions.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dboeditdatasourceactionoptionswidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/storeddbodatasource.cpp"
)
