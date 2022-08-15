
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/job.h"
        "${CMAKE_CURRENT_LIST_DIR}/jobmanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentreaddbjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/allbuffercsvexportjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/buffercsvexportjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentdeletedbjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/insertbufferdbjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/updatebufferdbjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/readjsonfilejob.h"
        "${CMAKE_CURRENT_LIST_DIR}/jsonparsejob.h"
        "${CMAKE_CURRENT_LIST_DIR}/jsonmappingjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/jsonmappingstubsjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationsjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/asterixjsonmappingjob.h"
        "${CMAKE_CURRENT_LIST_DIR}/asterixpostprocessjob.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentreaddbjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbcontentdeletedbjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/allbuffercsvexportjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/buffercsvexportjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/insertbufferdbjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/updatebufferdbjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jobmanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/readjsonfilejob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonparsejob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonmappingjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/jsonmappingstubsjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createartasassociationsjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/createassociationsjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/asterixjsonmappingjob.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/asterixpostprocessjob.cpp"
)

IF (jASTERIX_FOUND)
	MESSAGE(STATUS "Including ASTERIX jobs")

target_sources(compass
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/asterixdecodejob.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/asterixdecodejob.cpp"
)
ENDIF()
