include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/presentconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/modeapresentconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/present.h"
        "${CMAKE_CURRENT_LIST_DIR}/falseconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/modeafalseconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/false.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/presentconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/modeapresentconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/present.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/falseconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/modeafalseconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/false.cpp"        
)


