
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/arraylist.h"
     #   "${CMAKE_CURRENT_LIST_DIR}/arraylistdecl.h"
     #   "${CMAKE_CURRENT_LIST_DIR}/arraylistdef.h"
        "${CMAKE_CURRENT_LIST_DIR}/buffer.h"
     #   "${CMAKE_CURRENT_LIST_DIR}/bufferdecl.h"
     #   "${CMAKE_CURRENT_LIST_DIR}/bufferdef.h"
    #        src/buffer/bufferfilter.h
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/arraylist.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/buffer.cpp"
    #        src/buffer/bufferfilter.cpp
)


