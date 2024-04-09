include("${CMAKE_CURRENT_LIST_DIR}/complex/CMakeLists.txt")

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
        "${CMAKE_CURRENT_LIST_DIR}/simplereconstructorwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/simplereconstructorassociationwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/simpleassociator.h"
        "${CMAKE_CURRENT_LIST_DIR}/referencecalculator.h"
        "${CMAKE_CURRENT_LIST_DIR}/simplereferencecalculator.h"
        "${CMAKE_CURRENT_LIST_DIR}/simplereferencecalculatorwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/simpleaccuracyestimator.h"
        "${CMAKE_CURRENT_LIST_DIR}/accuracyestimatorbase.h"

    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/reconstructortask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructortaskdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructortarget.cpp"
        #"${CMAKE_CURRENT_LIST_DIR}/calculatereferencesstatusdialog.cpp"
        #"${CMAKE_CURRENT_LIST_DIR}/calculatereferencestarget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/reconstructorbase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/simplereconstructor.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/simplereconstructorwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/simplereconstructorassociationwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/simpleassociator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/simplereferencecalculator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/simplereferencecalculatorwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/simpleaccuracyestimator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/accuracyestimatorbase.cpp"
)
