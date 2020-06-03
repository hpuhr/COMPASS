
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/latexcontent.h"
        "${CMAKE_CURRENT_LIST_DIR}/latexdocument.h"
        "${CMAKE_CURRENT_LIST_DIR}/latexsection.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/latexcontent.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/latexdocument.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/latexsection.cpp"
)


