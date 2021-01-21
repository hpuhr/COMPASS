include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/correctconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/correctconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/correct.h"
        "${CMAKE_CURRENT_LIST_DIR}/falseconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/identificationfalseconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/false.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/correctconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/correctconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/correct.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/falseconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/identificationfalseconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/false.cpp"
)


