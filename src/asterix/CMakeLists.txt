
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

MESSAGE(STATUS "Including ASTERIX code")

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/asterixconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/asterixframingcombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/asterixeditioncombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/asterixcategoryconfig.h"
#        "${CMAKE_CURRENT_LIST_DIR}/jsondatamappingwidget.h"
#        "${CMAKE_CURRENT_LIST_DIR}/jsonobjectparser.h"
#        "${CMAKE_CURRENT_LIST_DIR}/jsonobjectparserwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/asterixconfigwidget.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/jsondatamapping.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/jsondatamappingwidget.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/jsonobjectparser.cpp"
#        "${CMAKE_CURRENT_LIST_DIR}/jsonobjectparserwidget.cpp"
)


