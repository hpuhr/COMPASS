
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/falsesingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/falsejoined.h"
        "${CMAKE_CURRENT_LIST_DIR}/presentsingle.h"
        "${CMAKE_CURRENT_LIST_DIR}/presentjoined.h"        
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/falsesingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/falsejoined.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/presentsingle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/presentjoined.cpp"                
    )


