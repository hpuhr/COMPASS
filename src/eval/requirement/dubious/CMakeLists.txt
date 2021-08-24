include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dubioustrack.h"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustrackconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustrackconfigwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dubioustrack.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustrackconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustrackconfigwidget.cpp"
    )


