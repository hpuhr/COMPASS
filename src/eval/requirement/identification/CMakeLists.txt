include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/correctconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/identificationcorrectconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/correct.h"
        "${CMAKE_CURRENT_LIST_DIR}/falseconfig.h"
        "${CMAKE_CURRENT_LIST_DIR}/identificationfalseconfigwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/false.h"
        "${CMAKE_CURRENT_LIST_DIR}/correct_period.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/correctconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/identificationcorrectconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/correct.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/falseconfig.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/identificationfalseconfigwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/false.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/correct_period.cpp"
)


