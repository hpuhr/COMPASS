
include("${CMAKE_CURRENT_LIST_DIR}/report/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/resultmanager.h"
        #"${CMAKE_CURRENT_LIST_DIR}/pdfgenerator.h"
        #"${CMAKE_CURRENT_LIST_DIR}/pdfgeneratordialog.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/resultmanager.cpp"
        #"${CMAKE_CURRENT_LIST_DIR}/pdfgenerator.cpp"
        #"${CMAKE_CURRENT_LIST_DIR}/pdfgeneratordialog.cpp"
    )
