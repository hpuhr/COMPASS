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
        "${CMAKE_CURRENT_LIST_DIR}/probabilitybase.h"
        "${CMAKE_CURRENT_LIST_DIR}/probabilitybaseconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/probabilitybaseconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/intervalbase.h"
        "${CMAKE_CURRENT_LIST_DIR}/intervalbaseconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/intervalbaseconfigwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/base.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/baseconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/baseconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/probabilitybase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/probabilitybaseconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/probabilitybaseconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/intervalbase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/intervalbaseconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/intervalbaseconfigwidget.cpp"
    )
