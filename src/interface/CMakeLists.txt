
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbtableinfo.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbinterface.h"
#        "${CMAKE_CURRENT_LIST_DIR}/dbinterfacewidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbinterfaceinfowidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/sqlgenerator.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbinterface.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/dbinterfacewidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbinterfaceinfowidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sqlgenerator.cpp"
)


