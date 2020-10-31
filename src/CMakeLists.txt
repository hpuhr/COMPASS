
include("${CMAKE_CURRENT_LIST_DIR}/buffer/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/asterix/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/client/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/config/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/command/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/connection/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/interface/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/object/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/util/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/latex/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/gui/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/filter/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/job/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/json/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/projection/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/property/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/schema/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/sector/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/task/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/view/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/unit/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/eval/CMakeLists.txt")

include("${CMAKE_CURRENT_LIST_DIR}/test/CMakeLists.txt")

include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/atsdb.h"
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/atsdb.cpp"
)


