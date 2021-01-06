include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/comparisontype.h"
        "${CMAKE_CURRENT_LIST_DIR}/comparisontypecombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/base.h"
        "${CMAKE_CURRENT_LIST_DIR}/baseconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/baseconfigwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/base.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/baseconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/baseconfigwidget.cpp"
    )


