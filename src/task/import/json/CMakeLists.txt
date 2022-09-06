include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttask.h"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttaskdialog.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonimporttaskdialog.cpp"
)


