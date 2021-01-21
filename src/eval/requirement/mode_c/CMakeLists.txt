include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/falseconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/modecfalseconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/false.h"
        "${CMAKE_CURRENT_LIST_DIR}/presentconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/modecpresentconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/present.h"        
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/falseconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/modecfalseconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/false.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/presentconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/modecpresentconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/present.cpp"        
    )


