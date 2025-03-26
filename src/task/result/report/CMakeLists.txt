
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/section.h"
        "${CMAKE_CURRENT_LIST_DIR}/treeitem.h"
        "${CMAKE_CURRENT_LIST_DIR}/treemodel.h"
        "${CMAKE_CURRENT_LIST_DIR}/report.h"
        "${CMAKE_CURRENT_LIST_DIR}/reportwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/sectionid.h"
        "${CMAKE_CURRENT_LIST_DIR}/sectioncontent.h"
        "${CMAKE_CURRENT_LIST_DIR}/sectioncontenttext.h"
        "${CMAKE_CURRENT_LIST_DIR}/sectioncontenttable.h"
        "${CMAKE_CURRENT_LIST_DIR}/sectioncontentfigure.h"

    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/section.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/treeitem.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/treemodel.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/report.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reportwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sectionid.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sectioncontent.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sectioncontenttext.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sectioncontenttable.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sectioncontentfigure.cpp"
    )
