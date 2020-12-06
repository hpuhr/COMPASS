include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dataconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/dataconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/data.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dataconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dataconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/data.cpp"
)


