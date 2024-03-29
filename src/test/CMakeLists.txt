
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_common.h"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_testable.h"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_conversions.h"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_event_injections.h"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_widget_setters.h"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_widget_getters.h"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_setget_native.h"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_setget.h"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_inject.h"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_find.h"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_cmd.h"

PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_event_injections.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_setget_native.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_setget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_inject.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_find.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ui_test_cmd.cpp"
)

set ( TEST_DATA_PATH "/home/sk/data/test/" )

    add_executable ( test_import_asterix "${CMAKE_CURRENT_LIST_DIR}/test_import_asterix.cpp")
    target_link_libraries ( test_import_asterix compass)

#add_executable ( test_import_json "${CMAKE_CURRENT_LIST_DIR}/test_import_json.cpp")
#target_link_libraries ( test_import_json compass)

enable_testing()

add_test(NAME TestImportASTERIX COMMAND test_import_asterix --data_path ${TEST_DATA_PATH} --filename 20190506.ff)

#add_test(NAME TestImportSDDLJSON COMMAND
#    test_import_json --data_path ${TEST_DATA_PATH} --filename sddl_10k.json --schema_name SDDL)

#add_test(NAME TestImportADSBExchangeJSON COMMAND
#    test_import_json --data_path ${TEST_DATA_PATH} --filename adsbexchange_small.zip --schema_name ADSBExchange)

#add_test(NAME TestImportOpenSkyNetworkJSON COMMAND
#    test_import_json --data_path ${TEST_DATA_PATH} --filename opensky.json.gz --schema_name OpenSkyNetwork)
