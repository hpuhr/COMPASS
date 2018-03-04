
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/structuredescription.h"
        "${CMAKE_CURRENT_LIST_DIR}/structuredescriptionmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/structureelement.h"
        "${CMAKE_CURRENT_LIST_DIR}/structurevariable.h"
    ##        src/struct/structuredescriptionplot.h
    ##        src/struct/structuredescriptionsystemtrack.h
    ##        src/struct/structuredescriptionreferencetrajectory.h
    ##        src/struct/structuredescriptionmlat.h
    ##        src/struct/structuredescriptionsensor.h
    #        src/struct/structurereader.h
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/structureelement.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/structuredescription.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/structuredescriptionmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/structurevariable.cpp"
    #        src/struct/structurereader.cpp
)


