
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/labelgenerator.h"
        "${CMAKE_CURRENT_LIST_DIR}/labelgeneratorwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/labeldswidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/labelcontentdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/labelplacement.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/labelgenerator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/labelgeneratorwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/labeldswidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/labelcontentdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/labelplacement.cpp"
)


