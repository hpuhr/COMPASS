
include_directories (
    "${CMAKE_CURRENT_LIST_DIR}"
    )

target_sources(atsdb
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/dbfilter.h"
        "${CMAKE_CURRENT_LIST_DIR}/filtermanager.h"
        "${CMAKE_CURRENT_LIST_DIR}/filtermanagerwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbfiltercondition.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbfilterwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/filtereditwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/filtergeneratorwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/filterconditionoperatorcombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/filterconditionresetvaluecombobox.h"
        "${CMAKE_CURRENT_LIST_DIR}/datasourcesfilter.h"
        "${CMAKE_CURRENT_LIST_DIR}/datasourcesfilterwidget.h"
        "${CMAKE_CURRENT_LIST_DIR}/dbospecificvaluesdbfilter.h"
        "${CMAKE_CURRENT_LIST_DIR}/utnfilter.h"
        "${CMAKE_CURRENT_LIST_DIR}/utnfilterwidget.h"
        #"${CMAKE_CURRENT_LIST_DIR}/dbospecificvaluesdbfilterwidget.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/dbfilter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbfiltercondition.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbfilterwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/filtermanager.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/filtermanagerwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/datasourcesfilter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/datasourcesfilterwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/filtereditwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/filtergeneratorwidget.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/dbospecificvaluesdbfilter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/utnfilter.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/utnfilterwidget.cpp"
        #"${CMAKE_CURRENT_LIST_DIR}/dbospecificvaluesdbfilterwidget.cpp"
)


