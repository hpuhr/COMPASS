
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/client.h"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow.h"
        "${CMAKE_CURRENT_LIST_DIR}/mainloadwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/client.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mainloadwidget.cpp"
)

add_executable ( compass_client "${CMAKE_CURRENT_LIST_DIR}/main.cpp")
target_link_libraries ( compass_client compass)


