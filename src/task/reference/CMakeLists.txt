include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/calculatereferencestask.h"
        "${CMAKE_CURRENT_LIST_DIR}/calculatereferencestaskdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/calculatereferencesstatusdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/calculatereferencesjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/calculatereferencestarget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/calculatereferencestask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/calculatereferencestaskdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/calculatereferencesstatusdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/calculatereferencesjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/calculatereferencestarget.cpp"
  
)
