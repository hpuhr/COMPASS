set(ATSDB_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
message("  Path: ${ATSDB_PATH}")

cmake_minimum_required(VERSION 3.1)
set ( CMAKE_BUILD_TYPE Debug )

project( ATSDB )
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "9")

message("  System: ${CMAKE_SYSTEM}")
SET(CMAKE_INSTALL_PREFIX ${ATSDB_PATH}/dist)
message("  Install Path: ${CMAKE_INSTALL_PREFIX}")

message("  Platform: Linux")
add_definitions ( -Wall -std=c++11 )

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5OpenGL)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

add_definitions(${Qt5Widgets_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

find_package ( Boost REQUIRED COMPONENTS regex system thread)
message("  Boost_INCLUDE_DIR: ${Boost_INCLUDE_DIR}")
message("  Boost_LIBRARY_DIR: ${Boost_LIBRARY_DIR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${ATSDB_PATH}/cmake_modules)
message("  CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}") 

find_package ( MySQLpp REQUIRED )
message("  MySQLpp_INCLUDE_DIR: ${MySQLpp_INCLUDE_DIR}") 
message("  MySQLpp_LIBRARY: ${MySQLpp_LIBRARY}") 

find_package ( MySQL REQUIRED )
message("  MYSQL_INCLUDE_DIR: ${MYSQL_INCLUDE_DIR}")
message("  MYSQL_LIBRARY: ${MYSQL_LIBRARY}")

find_package ( Sqlite3 REQUIRED )
message("  SQLite3_INCLUDE_DIR: ${SQLITE3_INCLUDE_DIR}")
message("  SQLite3_LIBRARY_DIR: ${SQLITE3_LIBRARIES}")

find_package ( LOG4CPP REQUIRED )
message("  LOG4CPP_INCLUDE_DIR: ${LOG4CPP_INCLUDE_DIR}")
message("  LOG4CPP_LIBRARY: ${LOG4CPP_LIBRARIES}") 

find_package ( GDAL REQUIRED )
message("  GDAL_INCLUDE_DIRS: ${GDAL_INCLUDE_DIRS}")
message("  GDAL_LIBRARIES: ${GDAL_LIBRARIES}")

find_package ( TinyXML2 REQUIRED )
message("  TINYXML2_INCLUDE_DIR: ${TINYXML2_INCLUDE_DIR}")
message("  TINYXML2_LIBRARY: ${TINYXML2_LIBRARY}") 

add_library(atsdb "")

add_definitions ( -DEXPERMENTAL_SRC=1 )
include(experimental_src/CMakeLists.txt)

include_directories (
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
    ${MySQLpp_INCLUDE_DIR}
    ${LOG4CPP_INCLUDE_DIR}
    ${GDAL_INCLUDE_DIRS}
    ${TINYXML2_INCLUDE_DIR}
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    .
    src
    src/buffer
    src/client
    src/config
    src/command
    src/connection
    src/interface
    src/object
    src/util
    src/filter
    src/job
    src/polygon
    src/projection
    src/property
    src/schema
    src/struct
    src/task
    src/variable
    src/view
    src/view/viewbase
    src/view/listboxview
    src/unit
    )

link_directories(
    ${Boost_LIBRARY_DIR}
    )

target_sources(atsdb
    PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/src/atsdb.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/buffer/arraylist.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/buffer/buffer.h"
    #        src/buffer/bufferfilter.h
    "${CMAKE_CURRENT_LIST_DIR}/src/client/client.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/client/mainwindow.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/client/managementwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/config/configurable.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/config/configuration.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/config/configurationmanager.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/command/dbcommand.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/command/dbcommandlist.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/dbconnection.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/mysqlppconnection.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/mysqlppconnectionwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/mysqlppconnectioninfowidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/dbresult.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/mysqlserver.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/mysqlserverwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/sqlitefile.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/sqliteconnection.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/sqliteconnectionwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/sqliteconnectioninfowidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/bufferwriter.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/dbfilter.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/sensorfilter.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/filtermanager.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/filtermanagerwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/dbfiltercondition.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/dbfilterwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/filtereditwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/filtergeneratorwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/filterconditionoperatorcombobox.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/filterconditionresetvaluecombobox.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/sensorfilterwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/interface/dbtableinfo.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/interface/dbinterface.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/interface/dbinterfacewidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/interface/dbinterfaceinfowidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/interface/sqlgenerator.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/job.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/jobmanager.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/jobmanagerwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/dboreaddbjob.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/buffercsvexportjob.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/dboactivedatasourcesdbjob.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/dbominmaxdbjob.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/finalizedboreadjob.h"
    #        src/job/dbovariabledistinctstatisticsdbjob.h
    #        src/job/dbocountdbjob.h
    #        src/job/dboinfodbjob.h
    "${CMAKE_CURRENT_LIST_DIR}/src/job/updatebufferdbjob.h"
    #        src/job/writebufferdbjob.h
    #        src/job/transformationjob.h
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobject.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobjectwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobjectinfowidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobjectcombobox.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariablewidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobjectmanager.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobjectmanagerwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobjectmanagerloadwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariabledatatypecombobox.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariableselectionwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariabledatatypecombobox.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbodatasource.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbodatasourcedefinitionwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariable.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariableset.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariableorderedset.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariableorderedsetwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/metadbovariable.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/metadbovariablewidget.h"
    #        src/object/dbovariablecombobox.h
    #        src/object/dbovariablesetwidget.h
    #        src/object/propertydatatypecombobox.h
    "${CMAKE_CURRENT_LIST_DIR}/src/object/stringrepresentationcombobox.h"
    #        src/polygon/airspacesectormanagerwidget.h
    #        src/polygon/airspacesectortreewidget.h
    #        src/polygon/airspacesectorwidget.h
    #        src/polygon/airspacesector.h
    #        src/polygon/airspacesectorparser.h
    #        src/polygon/airspacesectormanager.h
    #        src/polygon/geographicpoint.h
    #        src/polygon/polygon4ddi.h
    #        src/polygon/vector2.h
    "${CMAKE_CURRENT_LIST_DIR}/src/projection/projectionmanager.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/projection/projectionmanagerwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/property/property.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/property/propertylist.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/struct/structuredescription.h"
    ##        src/struct/structuredescriptionplot.h
    ##        src/struct/structuredescriptionsystemtrack.h
    ##        src/struct/structuredescriptionreferencetrajectory.h
    ##        src/struct/structuredescriptionmlat.h
    ##        src/struct/structuredescriptionsensor.h
    "${CMAKE_CURRENT_LIST_DIR}/src/struct/structuredescriptionmanager.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/struct/structureelement.h"
    #        src/struct/structurereader.h
    "${CMAKE_CURRENT_LIST_DIR}/src/struct/structurevariable.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbtable.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbtablecolumn.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/metadbtable.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbtablecolumncombobox.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbschema.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbschemawidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbschemamanager.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbschemamanagerwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbtablewidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/metadbtablewidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/task/taskmanager.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/task/radarplotpositioncalculatortask.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/task/radarplotpositioncalculatortaskwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/unit/unitselectionwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewmanager.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewmanagerwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewcontainer.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewcontainerwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewcontainerconfigwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/eventprocessor.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/view.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/viewwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/viewsettings.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/viewmodel.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/viewselection.h"
#    src/view/viewbase/dbviewmodel.h
#    src/view/viewbase/dbview.h
#    src/view/viewbase/dbviewwidget.h
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/buffertablemodel.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/buffertablewidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/listboxviewdatasource.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/listboxview.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/listboxviewconfigwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/listboxviewdatawidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/listboxviewwidget.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/unit/unit.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/unit/unitmanager.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/unit/dimension.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/util/config.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/util/global.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/util/number.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/util/stringconv.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/util/singleton.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/util/logger.h"
    )


target_sources(atsdb
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/src/atsdb.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/buffer/arraylist.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/buffer/buffer.cpp"
    #        src/buffer/bufferfilter.cpp
    "${CMAKE_CURRENT_LIST_DIR}/src/client/client.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/client/mainwindow.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/client/managementwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/config/configurable.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/config/configuration.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/config/configurationmanager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/bufferwriter.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/mysqlppconnection.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/mysqlppconnectionwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/mysqlppconnectioninfowidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/mysqlserverwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/sqliteconnection.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/sqliteconnectionwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/connection/sqliteconnectioninfowidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/dbfilter.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/dbfiltercondition.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/dbfilterwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/filtermanager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/filtermanagerwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/sensorfilter.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/sensorfilterwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/filtereditwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/filter/filtergeneratorwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/interface/dbinterface.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/interface/dbinterfacewidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/interface/dbinterfaceinfowidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/interface/sqlgenerator.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/dboactivedatasourcesdbjob.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/dbominmaxdbjob.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/dboreaddbjob.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/finalizedboreadjob.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/buffercsvexportjob.cpp"
    #        src/job/dbovariabledistinctstatisticsdbjob.cpp
    #        src/job/dbocountdbjob.cpp
    #        src/job/dboinfodbjob.cpp
    "${CMAKE_CURRENT_LIST_DIR}/src/job/updatebufferdbjob.cpp"
    #        src/job/writebufferdbjob.cpp
    #        src/job/transformationjob.cpp
    "${CMAKE_CURRENT_LIST_DIR}/src/job/jobmanager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/job/jobmanagerwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobject.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobjectwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobjectinfowidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobjectmanager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobjectmanagerwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbobjectmanagerloadwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbodatasource.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbodatasourcedefinitionwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariable.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariablewidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariableset.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariableorderedset.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariableorderedsetwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/dbovariableselectionwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/metadbovariable.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/object/metadbovariablewidget.cpp"
    #        src/object/dbovariablecombobox.cpp
    #        src/object/dbovariablesetwidget.cpp
    #        src/polygon/polygon4ddi.cpp
    #        src/polygon/geographicpoint.cpp
    #        src/polygon/airspacesector.cpp
    #        src/polygon/airspacesectormanager.cpp
    #        src/polygon/airspacesectormanagerwidget.cpp
    #        src/polygon/airspacesectortreewidget.cpp
    #        src/polygon/airspacesectorwidget.cpp
    "${CMAKE_CURRENT_LIST_DIR}/src/projection/projectionmanager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/projection/projectionmanagerwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/property/property.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/task/taskmanager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/task/radarplotpositioncalculatortask.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/task/radarplotpositioncalculatortaskwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbschemawidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbtablewidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbtablecolumncombobox.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbtable.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbtablecolumn.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbschema.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbschemamanager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/dbschemamanagerwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/metadbtable.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/schema/metadbtablewidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/struct/structureelement.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/struct/structuredescription.cpp"
    #        src/struct/structurereader.cpp
    "${CMAKE_CURRENT_LIST_DIR}/src/struct/structuredescriptionmanager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/struct/structurevariable.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/unit/dimension.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/unit/unitselectionwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/unit/unitmanager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/util/config.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/util/logger.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/util/number.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/util/stringconv.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewmanager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewmanagerwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewcontainer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewcontainerwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/eventprocessor.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/view.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/viewwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewcontainerconfigwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/viewmodel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/viewsettings.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/viewbase/viewselection.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/buffertablemodel.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/buffertablewidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/listboxview.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/listboxviewdatasource.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/listboxviewconfigwidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/listboxviewdatawidget.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/view/listboxview/listboxviewwidget.cpp"
#    src/view/viewbase/dbviewmodel.cpp
#    src/view/viewbase/dbview.cpp
#    src/view/viewbase/dbviewwidget.cpp
    )

target_link_libraries ( atsdb
    Qt5::Widgets
    Qt5::Core
    Qt5::OpenGL
    ${TINYXML2_LIBRARY}
    ${Boost_LIBRARIES}
    ${LOG4CPP_LIBRARIES}
    ${MySQLpp_LIBRARY}
    ${MYSQL_LIBRARY}
    ${GDAL_LIBRARIES}
    ${SQLITE3_LIBRARIES})

set ( client_SRCS
    src/client/main.cpp
    )

add_executable ( atsdb_client ${client_SRCS})
target_link_libraries ( atsdb_client
    atsdb)

set ( LIBRARY_OUTPUT_PATH dist/lib )
#INSTALL_FILES(/include .h ${atsdb_HDRS})

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
        COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
endif(DOXYGEN_FOUND)

file(GLOB DOT ".*")
file(GLOB TILD "*~")
file(GLOB TEMP "*.cmake")

set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-src")
set(CPACK_SOURCE_IGNORE_FILES
    "/CMakeFiles/"
    "/_CPack_Packages/"
    "/dist/"
    "/.git/"
    "CMakeCache\\\\.txt"
    ".gz$"
    "install_manifest.txt"
    "Makefile$"
    "ui_.*$"
    "${DOT};${TILD};${TEMP}"
    "${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)
message("CPACK_SOURCE_IGNORE_FILES = ${CPACK_SOURCE_IGNORE_FILES}")

set(CPACK_BIN_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-bin")

add_custom_target(package_binary COMMAND
    ${CMAKE_COMMAND} -E tar "cfvz" "${CPACK_BIN_PACKAGE_FILE_NAME}.tar.gz" "dist/" "readme*.txt")
add_dependencies(package_binary atsdb)





