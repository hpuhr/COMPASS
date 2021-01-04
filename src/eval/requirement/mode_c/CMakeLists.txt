include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/falseconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/falseconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/false.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/falseconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/falseconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/false.cpp"
)


