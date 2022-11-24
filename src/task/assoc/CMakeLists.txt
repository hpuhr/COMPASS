include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsstatusdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationstask.h"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationstaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationstaskdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationsstatusdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationsjob.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsstatusdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationstaskdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationstask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationstaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationstaskdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationsstatusdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationsjob.cpp"        
)
