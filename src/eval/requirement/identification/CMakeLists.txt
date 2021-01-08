include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/correctconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/correctconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/correct.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/correctconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/correctconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/correct.cpp"
)


