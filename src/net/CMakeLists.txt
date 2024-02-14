
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/udpreceiver.h"
        "${CMAKE_CURRENT_LIST_DIR}/tcpserver.h"
        "${CMAKE_CURRENT_LIST_DIR}/packetsniffer.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/udpreceiver.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/tcpserver.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/packetsniffer.cpp"
)
