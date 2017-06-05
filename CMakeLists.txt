set(ATSDB_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
message("  Path: ${ATSDB_PATH}")

cmake_minimum_required( VERSION 2.6 )
set ( CMAKE_BUILD_TYPE Debug )

project( ATSDB )

message("  System: ${CMAKE_SYSTEM}")
SET(CMAKE_INSTALL_PREFIX ${ATSDB_PATH}/dist)
message("  Install Path: ${CMAKE_INSTALL_PREFIX}")

message("  Platform: Linux")
add_definitions ( -Wall -std=c++11 )

find_package ( Qt4 REQUIRED )
message("  QT_QTCORE_LIBRARY: ${QT_QTCORE_LIBRARY}") 
include ( ${QT_USE_FILE} )

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
#message("  MYSQL_LIBRARY: ${MYSQL_LIBRARY}")

#find_package ( Sqlite3 REQUIRED )
#message("  SQLite3_INCLUDE_DIR: ${SQLITE3_INCLUDE_DIR}")
#message("  SQLite3_LIBRARY_DIR: ${SQLITE3_LIBRARIES}") 

find_package ( LOG4CPP REQUIRED )
message("  LOG4CPP_INCLUDE_DIR: ${LOG4CPP_INCLUDE_DIR}")
message("  LOG4CPP_LIBRARY: ${LOG4CPP_LIBRARIES}") 

find_package ( GDAL REQUIRED )
message("  GDAL_INCLUDE_DIRS: ${GDAL_INCLUDE_DIRS}")
message("  GDAL_LIBRARIES: ${GDAL_LIBRARIES}")

find_package ( TinyXML2 REQUIRED )
message("  TINYXML2_INCLUDE_DIR: ${TINYXML2_INCLUDE_DIR}")
message("  TINYXML2_LIBRARY: ${TINYXML2_LIBRARY}") 

find_package ( GDAL REQUIRED )
message("  GDAL_INCLUDE_DIRS: ${GDAL_INCLUDE_DIRS}")
message("  GDAL_LIBRARIES: ${GDAL_LIBRARIES}")

include_directories (
	${CMAKE_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR}
	${QT_QTGUI_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	#${SQLITE3_INCLUDE_DIR}
	${MySQLpp_INCLUDE_DIR}
        ${MYSQL_INCLUDE_DIR}
	${LOG4CPP_INCLUDE_DIR}
	${GDAL_INCLUDE_DIRS}
	${TINYXML2_INCLUDE_DIR}
	.
	src
        src/buffer
        src/client
        src/computation
        src/config
	src/command
	src/connection
	src/datasource
        src/gui
        src/interface
	src/mem
	src/object
	src/util
	src/filter
	src/job
	src/polygon
	src/polygon/acgxml
	src/polygon/shapefile
        src/projection
	src/property
        src/result
        src/schema
        src/struct
        src/thread
        src/transform
        src/variable
	src/unit
    )

link_directories(
    ${Boost_LIBRARY_DIR}
    )
	
set ( LIBRARY_OUTPUT_PATH dist/lib )

set ( atsdb_HDRS
        src/atsdb.h
        src/buffer/arraylist.h
        src/buffer/buffer.h
#        src/buffer/bufferfilter.h
        src/client/client.h
        src/config/configurable.h
        src/config/configuration.h
        src/config/configurationmanager.h
        src/command/dbcommand.h
        src/command/dbcommandlist.h
        src/connection/mysqlppconnection.h
#        src/connection/bufferwriter.h
        src/connection/dbresult.h
#        src/connection/mysqlconconnection.h
        src/connection/mysqlserver.h
        src/connection/sqlgenerator.h
##        src/connection/sqliteconnection.h
#        src/datasource/datasource.h
#        src/filter/dbfilter.h
#        src/filter/sensorfilter.h
#        src/filter/filtermanager.h
#        src/job/dbjob.h
        src/job/job.h
        src/job/joborderer.h
#        src/job/dboactivedatasourcesdbjob.h
#        src/job/dbovariabledistinctstatisticsdbjob.h
#        src/job/dbovariableminmaxdbjob.h
#        src/job/finalizedboreadjob.h
#        src/job/dbocountdbjob.h
#        src/job/dboreaddbjob.h
#        src/job/dboinfodbjob.h
#        src/job/updatebufferdbjob.h
#        src/job/writebufferdbjob.h
#        src/job/postprocessdbjob.h
#        src/job/transformationjob.h
        src/object/dbobject.h
        src/object/dbobjectmanager.h
        src/object/dbovariable.h
        src/object/dbovariableset.h
        src/object/dbovariableorderedset.h
#        src/polygon/airspacesector.h
#        src/polygon/airspacesectorparser.h
#        src/polygon/airspacesectormanager.h
#        src/polygon/geographicpoint.h
#        src/polygon/polygon4ddi.h
#        src/polygon/vector2.h
#        src/polygon/acgxml/acgxmlparser.h
#        src/polygon/acgxml/abd.h
#        src/polygon/acgxml/ase.h
#        src/polygon/shapefile/shapefileparser.h
#        src/projection/projectionmanager.h
        src/property/property.h
        src/property/propertylist.h
#        src/property/propertyarray.h
#        src/property/propertycontainer.h
#        src/result/bufferset.h
#        src/result/bufferreceiver.h
        src/schema/dbtable.h
        src/schema/dbtablecolumn.h
        src/schema/dbschema.h
        src/schema/dbschemamanager.h
        src/schema/metadbtable.h
        src/struct/structuredescription.h
##        src/struct/structuredescriptionplot.h
##        src/struct/structuredescriptionsystemtrack.h
##        src/struct/structuredescriptionreferencetrajectory.h
##        src/struct/structuredescriptionmlat.h
##        src/struct/structuredescriptionsensor.h
        src/struct/structuredescriptionmanager.h
        src/struct/structureelement.h
#        src/struct/structurereader.h
        src/struct/structurevariable.h
        src/interface/dbtableinfo.h
        src/interface/dbinterface.h
#        src/thread/thread.h
#        src/thread/timedthread.h
#        src/thread/workerthread.h
#        src/thread/workerthreadmanager.h
#        src/computation/workflow.h
#        src/computation/computation.h
#        src/computation/computationelement.h
#        src/computation/computationpipeline.h
#        src/computation/computationpipelinequeue.h
#        src/computation/transformationbase.h
#        src/computation/transformation.h
#        src/computation/transformationvariable.h
#        src/computation/transformationfactory.h
#        src/transform/calculatehistogram.h
#        src/transform/calculatehistogram2d.h
#        src/transform/calculateposition.h
#        src/transform/calculateposition2d.h
#        src/transform/calculatelines.h
#        src/transform/calculatestyle.h
        src/unit/unit.h
        src/unit/unitmanager.h
        src/unit/quantity.h
        src/util/config.h
        src/util/global.h
#        src/util/data.h
        src/util/number.h
        src/util/stringconv.h
        src/util/singleton.h
        src/util/logger.h
        )

set (atsdb_MOC_HEADERS
        src/client/mainwindow.h
        src/connection/dbconnection.h
#        src/filter/dbfiltercondition.h
#        src/filter/dbfilterwidget.h
#        src/filter/filtereditwidget.h
#        src/filter/filtergeneratorwidget.h
#        src/filter/sensorfilterwidget.h
        src/connection/mysqlppconnectionwidget.h
        src/connection/mysqlserverwidget.h
        src/interface/dbinterfacewidget.h
#        src/gui/qcollapsablewidget.h
#        src/gui/workerthreadwidget.h
#        src/gui/workfloweditwidget.h
#        src/gui/workfloweditdialog.h
#        src/gui/workflowsettingseditdialog.h
#        src/gui/computationeditdialog.h
#        src/gui/transformationeditdialog.h
#        src/gui/transformationvariableeditdialog.h
#        src/gui/filtereditdialog.h
#        src/schema/dbtablecolumncombobox.h
        src/schema/dbschemamanagerwidget.h
        src/schema/dbschemawidget.h
        src/schema/dbtablewidget.h
        src/schema/metadbtablewidget.h
        src/object/dbobjectmanagerwidget.h
        src/object/dbobjectwidget.h
#        src/object/dbovariablecombobox.h
#        src/object/dbovariablesetwidget.h
#        src/object/dbovariableorderedsetwidget.h
#        src/object/dbovariableselectionwidget.h
        src/object/dbovariabledatatypecombobox.h
#        src/object/propertydatatypecombobox.h
#        src/object/metadbobjecteditwidget.h
#        src/object/stringrepresentationcombobox.h
        src/unit/unitselectionwidget.h
#        src/polygon/airspacesectormanagerwidget.h
#        src/polygon/airspacesectortreewidget.h
#        src/polygon/airspacesectorwidget.h
#        src/projection/projectionmanagerwidget.h
        )

set ( atsdb_SRCS
        src/atsdb.cpp
        src/buffer/arraylist.cpp
        src/buffer/buffer.cpp
#        src/buffer/bufferfilter.cpp
        src/client/client.cpp
        src/client/mainwindow.cpp
        src/config/configurable.cpp
        src/config/configuration.cpp
        src/config/configurationmanager.cpp
#        src/connection/bufferwriter.cpp
#        src/connection/mysqlconconnection.cpp
        src/connection/mysqlppconnection.cpp
        src/connection/mysqlppconnectionwidget.cpp
        src/connection/mysqlserverwidget.cpp
        src/connection/sqlgenerator.cpp
#        src/connection/sqliteconnection.cpp
#        src/datasource/datasource.cpp
#        src/filter/dbfilter.cpp
#        src/filter/dbfiltercondition.cpp
#        src/filter/dbfilterwidget.cpp
#        src/filter/filtermanager.cpp
#        src/filter/sensorfilter.cpp
#        src/filter/sensorfilterwidget.cpp
#        src/filter/filtereditwidget.cpp
#        src/filter/filtergeneratorwidget.cpp
#        src/gui/qcollapsablewidget.cpp
#        src/gui/workerthreadwidget.cpp
#        src/gui/workfloweditwidget.cpp
#        src/gui/workfloweditdialog.cpp
#        src/gui/workflowsettingseditdialog.cpp
#        src/gui/computationeditdialog.cpp
#        src/gui/transformationeditdialog.cpp
#        src/gui/transformationvariableeditdialog.cpp
#        src/gui/filtereditdialog.cpp
        src/interface/dbinterface.cpp
        src/interface/dbinterfacewidget.cpp
#        src/job/dboactivedatasourcesdbjob.cpp
#        src/job/dbovariabledistinctstatisticsdbjob.cpp
#        src/job/dbovariableminmaxdbjob.cpp
#        src/job/dbocountdbjob.cpp
#        src/job/dboreaddbjob.cpp
#        src/job/dboinfodbjob.cpp
#        src/job/dbjob.cpp
        src/job/job.cpp
        src/job/joborderer.cpp
#        src/job/finalizedboreadjob.cpp
#        src/job/updatebufferdbjob.cpp
#        src/job/writebufferdbjob.cpp
#        src/job/postprocessdbjob.cpp
#        src/job/transformationjob.cpp
#        src/object/dbovariablecombobox.cpp
#        src/object/dbovariablesetwidget.cpp
#        src/object/dbovariableorderedsetwidget.cpp
#        src/object/dbovariableselectionwidget.cpp
        src/object/dbobjectwidget.cpp
        src/object/dbobjectmanagerwidget.cpp
#        src/object/metadbobjecteditwidget.cpp
        src/object/dbobject.cpp
        src/object/dbobjectmanager.cpp
        src/object/dbovariable.cpp
        src/object/dbovariableset.cpp
        src/object/dbovariableorderedset.cpp
#        src/polygon/polygon4ddi.cpp
#        src/polygon/geographicpoint.cpp
#        src/polygon/airspacesector.cpp
#        src/polygon/airspacesectormanager.cpp
#        src/polygon/airspacesectormanagerwidget.cpp
#        src/polygon/airspacesectortreewidget.cpp
#        src/polygon/airspacesectorwidget.cpp
#        src/polygon/acgxml/acgxmlparser.cpp
#        src/polygon/acgxml/abd.cpp
#        src/polygon/acgxml/ase.cpp
#        src/polygon/shapefile/shapefileparser.cpp
#        src/projection/projectionmanager.cpp
#        src/projection/projectionmanagerwidget.cpp
        src/property/property.cpp
#        src/property/propertycontainer.cpp
#        src/result/bufferset.cpp
        src/schema/dbschemawidget.cpp
        src/schema/dbtablewidget.cpp
#        src/schema/dbtablecolumncombobox.cpp
        src/schema/dbtable.cpp
        src/schema/dbtablecolumn.cpp
        src/schema/dbschema.cpp
        src/schema/dbschemamanager.cpp
        src/schema/dbschemamanagerwidget.cpp
        src/schema/metadbtable.cpp
        src/schema/metadbtablewidget.cpp
        src/struct/structureelement.cpp
        src/struct/structuredescription.cpp
#        src/struct/structurereader.cpp
        src/struct/structuredescriptionmanager.cpp
        src/struct/structurevariable.cpp
#        src/thread/thread.cpp
#        src/thread/timedthread.cpp
#        src/thread/workerthread.cpp
#        src/thread/workerthreadmanager.cpp
#        src/computation/workflow.cpp
#        src/computation/computation.cpp
#        src/computation/computationelement.cpp
#        src/computation/computationpipeline.cpp
#        src/computation/computationpipelinequeue.cpp
#        src/computation/transformationbase.cpp
#        src/computation/transformation.cpp
#        src/computation/transformationvariable.cpp
#        src/computation/transformationfactory.cpp
#        src/transform/calculatehistogram.cpp
#        src/transform/calculatehistogram2d.cpp
#        src/transform/calculateposition.cpp
#        src/transform/calculateposition2d.cpp
#        src/transform/calculatelines.cpp
#        src/transform/calculatestyle.cpp
        src/unit/quantity.cpp
        src/unit/unitselectionwidget.cpp
        src/unit/unitmanager.cpp
        src/util/config.cpp
        src/util/logger.cpp
#        src/util/data.cpp
        src/util/number.cpp
        src/util/stringconv.cpp
        )

set ( atsdb_UI
 #       src/gui/filtereditdialogbase.ui
 #       src/gui/computationeditdialogbase.ui
 #       src/gui/transformationeditdialogbase.ui
 #       src/gui/transformationvariableeditdialogbase.ui
 #       src/gui/workflowsettingseditdialogbase.ui
        )

set ( client_SRCS
        src/client/main.cpp
    )

	
QT4_WRAP_UI( UI_HDRS ${atsdb_UI} )
qt4_wrap_cpp (atsdb_MOC_OUTFILES ${atsdb_MOC_HEADERS})
add_library ( atsdb SHARED ${atsdb_SRCS} ${atsdb_HDRS} ${UIS} ${UI_HDRS} ${RSCS} ${TRS} ${atsdb_MOC_OUTFILES} )
target_link_libraries ( atsdb  ${GDAL_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${TINYXML2_LIBRARY} ${GDAL_LIBRARIES} ${Boost_LIBRARIES} ${LOG4CPP_LIBRARIES} ${MySQLpp_LIBRARY})
# ${SQLITE3_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_DATA_TIME_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_IOSTREAMS_LIBRARY}

add_executable ( atsdb_client ${atsdb_HDRS} ${client_SRCS})
target_link_libraries ( atsdb_client atsdb ${GDAL_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${TINYXML2_LIBRARY} ${GDAL_LIBRARIES} ${Boost_LIBRARIES} ${LOG4CPP_LIBRARIES} ${MySQLpp_LIBRARY})

INSTALL_FILES(/include .h ${atsdb_HDRS})
INSTALL_FILES(/include .h ${atsdb_MOC_HEADERS})
INSTALL_FILES(/include .h ${UI_HDRS})
 #INSTALL_FILES(/include .h ${atsdb_HDRS})
#INSTALL_FILES(/include .h ${atsdb_MOC_HEADERS})
#INSTALL_FILES(/include .h ${UI_HDRS})  

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




        
