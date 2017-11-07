
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/eventprocessor.h"
        "${CMAKE_CURRENT_LIST_DIR}/view.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewsettings.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewmodel.h"
        "${CMAKE_CURRENT_LIST_DIR}/viewselection.h"
#    dbviewmodel.h
#    dbview.h
#    dbviewwidget.h
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/eventprocessor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/view.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewmodel.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewsettings.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/viewselection.cpp"
        #    src/view/viewbase/dbviewmodel.cpp
        #    src/view/viewbase/dbview.cpp
        #    src/view/viewbase/dbviewwidget.cpp
)


