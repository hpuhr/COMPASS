include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/reconstructortask.h"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructortaskdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructortarget.h"
        #"${CMAKE_CURRENT_LIST_DIR}/calculatereferencesstatusdialog.h"
        #"${CMAKE_CURRENT_LIST_DIR}/calculatereferencestarget.h"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructorbase.h"
        "${CMAKE_CURRENT_LIST_DIR}/simplereconstructor.h"
        "${CMAKE_CURRENT_LIST_DIR}/simpleassociator.h"

    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/reconstructortask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructortaskdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructortarget.cpp"
        #"${CMAKE_CURRENT_LIST_DIR}/calculatereferencesstatusdialog.cpp"
        #"${CMAKE_CURRENT_LIST_DIR}/calculatereferencestarget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructorbase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/simplereconstructor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/simpleassociator.cpp"
)
