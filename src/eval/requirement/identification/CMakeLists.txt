include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/identificationconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/identificationconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/identification.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/identificationconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/identificationconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/identification.cpp"
)


