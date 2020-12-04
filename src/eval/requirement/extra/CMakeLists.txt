include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/utnsconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/utnsconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/utns.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/utnsconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/utnsconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/utns.cpp"
)


