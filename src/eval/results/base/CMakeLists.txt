
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/base.h"
        "${CMAKE_CURRENT_LIST_DIR}/single.h"
        "${CMAKE_CURRENT_LIST_DIR}/joined.h"
        "${CMAKE_CURRENT_LIST_DIR}/presentbase.h"
        "${CMAKE_CURRENT_LIST_DIR}/falsebase.h"
        "${CMAKE_CURRENT_LIST_DIR}/correctbase.h"
        "${CMAKE_CURRENT_LIST_DIR}/intervalbase.h"
        "${CMAKE_CURRENT_LIST_DIR}/probabilitybase.h"
        "${CMAKE_CURRENT_LIST_DIR}/valueaccumulator.h"
        "${CMAKE_CURRENT_LIST_DIR}/annotations.h"
        "${CMAKE_CURRENT_LIST_DIR}/result_defs.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/base.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/single.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/joined.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/presentbase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/falsebase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/correctbase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/intervalbase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/probabilitybase.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/valueaccumulator.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/annotations.cpp"
    )
