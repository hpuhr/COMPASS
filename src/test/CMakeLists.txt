
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

#target_sources(atsdb
#    PUBLIC
#        "${CMAKE_CURRENT_LIST_DIR}/testclient.h"
#    PRIVATE
#        "${CMAKE_CURRENT_LIST_DIR}/testclient.cpp"
#)

IF (jASTERIX_FOUND)

add_executable ( test_import_asterix "${CMAKE_CURRENT_LIST_DIR}/test_import_asterix.cpp")
target_link_libraries ( test_import_asterix atsdb)

set ( TEST_DATA_PATH "/home/sk/data/test/" )

enable_testing()
add_test(NAME TestATSDB COMMAND test_import_asterix --data_path ${TEST_DATA_PATH} --filename 20190506.ff)

ENDIF()
