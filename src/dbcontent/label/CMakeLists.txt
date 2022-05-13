
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/labelgenerator.h"
        "${CMAKE_CURRENT_LIST_DIR}/labelgeneratorwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/labeldswidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/labelgenerator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/labelgeneratorwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/labeldswidget.cpp"
)


