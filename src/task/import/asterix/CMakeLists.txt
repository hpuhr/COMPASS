include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttask.h"
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttaskwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttaskdialog.h"
        "${CMAKE_CURRENT_LIST_DIR}/asterixjsonmappingjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/asterixpostprocessjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/asterixudpreceiver.h"
        "${CMAKE_CURRENT_LIST_DIR}/asterixdecodejob.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttask.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttaskwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/asteriximporttaskdialog.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/asterixjsonmappingjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/asterixpostprocessjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/asterixudpreceiver.cpp"        
        "${CMAKE_CURRENT_LIST_DIR}/asterixdecodejob.cpp"
)

