
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

#target_sources(atsdb
#    PUBLIC
#        "${CMAKE_CURRENT_LIST_DIR}/testclient.h"
#    PRIVATE
#        "${CMAKE_CURRENT_LIST_DIR}/testclient.cpp"
#)

add_executable ( test_atsdb "${CMAKE_CURRENT_LIST_DIR}/test_atsdb.cpp")
target_link_libraries ( test_atsdb atsdb)

enable_testing()
add_test(NAME TestATSDB COMMAND test_atsdb)
