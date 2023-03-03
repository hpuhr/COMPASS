
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dubiousbase.h"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustracksingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustrackjoined.h"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustargetsingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustargetjoined.h"  
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dubiousbase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustracksingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustrackjoined.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustargetsingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dubioustargetjoined.cpp" 
    )


