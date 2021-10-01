
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dubioustracksingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustrackjoined.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dubioustracksingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustrackjoined.cpp"
    )


