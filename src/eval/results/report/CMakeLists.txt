
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/treeitem.h"
        "${CMAKE_CURRENT_LIST_DIR}/treemodel.h"
        "${CMAKE_CURRENT_LIST_DIR}/rootitem.h"
        "${CMAKE_CURRENT_LIST_DIR}/section.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/treeitem.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/treemodel.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/rootitem.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/section.cpp"
    )


