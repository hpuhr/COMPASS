
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/client.h"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow.h"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow_commands.h"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow_commands_file.h"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow_commands_import.h"
        "${CMAKE_CURRENT_LIST_DIR}/appmode.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/client.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow_commands.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow_commands_file.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/mainwindow_commands_import.cpp"        
)

add_executable ( compass_client "${CMAKE_CURRENT_LIST_DIR}/main.cpp")
target_link_libraries ( compass_client compass)


