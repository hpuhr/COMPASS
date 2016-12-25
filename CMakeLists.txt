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
message("  MYSQL_LIBRARY: ${MYSQL_LIBRARY}") 

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
	src/config
	src/struct
	src/schema
	src/command
	src/connection
	src/datasource
	src/result
	src/mem
	src/object
	src/util
	src/filter
	src/job
	src/gui
	src/polygon
	src/polygon/acgxml
	src/polygon/shapefile
        src/projection
	src/property
	src/buffer
	src/variable
	src/thread
        src/computation
        src/transform
	src/unit
    )

link_directories(
    ${Boost_LIBRARY_DIR}
    )
	
set ( LIBRARY_OUTPUT_PATH dist/lib )

set ( atsdb_HDRS
#        src/ATSDB.h
        src/buffer/ArrayList.h
#        src/buffer/ArrayContainer.h
        src/buffer/Buffer.h
#        src/buffer/BufferFilter.h
        src/config/Configurable.h
        src/config/Configuration.h
        src/config/ConfigurationManager.h
        src/command/DBCommand.h
        src/command/DBCommandList.h
        src/connection/BufferWriter.h
        src/connection/DBConnection.h
        src/connection/DBConnectionInfo.h
#        src/connection/MySQLConConnection.h
        src/connection/MySQLppConnection.h
##        src/connection/SQLiteConnection.h
        src/datasource/DataSource.h
        src/filter/DBFilter.h
        src/filter/SensorFilter.h
        src/filter/FilterManager.h
#        src/job/DBJob.h
#        src/job/Job.h
#        src/job/JobOrderer.h
#        src/job/DBOActiveDataSourcesDBJob.h
#        src/job/DBOVariableDistinctStatisticsDBJob.h
#        src/job/DBOVariableMinMaxDBJob.h
#        src/job/FinalizeDBOReadJob.h
#        src/job/DBOCountDBJob.h
#        src/job/DBOReadDBJob.h
#        src/job/DBOInfoDBJob.h
#        src/job/UpdateBufferDBJob.h
#        src/job/WriteBufferDBJob.h
#        src/job/PostProcessDBJob.h
#        src/job/TransformationJob.h
#        src/mem/ArrayTemplateManager.h
#        src/mem/ArrayTemplate.h
#        src/mem/BufferTemplate.h
#        src/mem/MemoryManager.h
        src/object/DBObject.h
        src/object/DBObjectManager.h
        src/object/DBOVariable.h
        src/object/DBOVariableSet.h
        src/object/DBOVariableOrderedSet.h
        src/polygon/AirspaceSector.h
        src/polygon/AirspaceSectorParser.h
        src/polygon/AirspaceSectorManager.h
        src/polygon/GeographicPoint.h
        src/polygon/Polygon4DDI.h
        src/polygon/Vector2.h
#        src/polygon/acgxml/ACGXMLParser.h
#        src/polygon/acgxml/Abd.h
#        src/polygon/acgxml/Ase.h
        src/polygon/shapefile/ShapeFileParser.h
        src/projection/ProjectionManager.h
        src/property/Property.h
        src/property/PropertyList.h
#        src/property/PropertyArray.h
#        src/property/PropertyContainer.h
        src/result/BufferSet.h
        src/result/DBResult.h
#        src/result/BufferReceiver.h
#        src/struct/StructureDescription.h
##        src/struct/StructureDescriptionPlot.h
##        src/struct/StructureDescriptionSystemTrack.h
##        src/struct/StructureDescriptionReferenceTrajectory.h
##        src/struct/StructureDescriptionMLAT.h
##        src/struct/StructureDescriptionSensor.h
#        src/struct/StructureDescriptionManager.h
#        src/struct/StructureElement.h
#        src/struct/StructureReader.h
#        src/struct/StructureVariable.h
        src/schema/DBTable.h
        src/schema/DBTableColumn.h
        src/schema/DBSchema.h
        src/schema/DBSchemaManager.h
        src/schema/MetaDBTable.h
        src/schema/SQLGenerator.h
        src/thread/Thread.h
        src/thread/TimedThread.h
        src/thread/WorkerThread.h
        src/thread/WorkerThreadManager.h
#        src/computation/Workflow.h
#        src/computation/Computation.h
#        src/computation/ComputationElement.h
#        src/computation/ComputationPipeline.h
#        src/computation/ComputationPipelineQueue.h
#        src/computation/TransformationBase.h
#        src/computation/Transformation.h
#        src/computation/TransformationVariable.h
#        src/computation/TransformationFactory.h
#        src/transform/CalculateHistogram.h
#        src/transform/CalculateHistogram2D.h
#        src/transform/CalculatePosition.h
#        src/transform/CalculatePosition2D.h
#        src/transform/CalculateLines.h
#        src/transform/CalculateStyle.h
        src/unit/Unit.h
        src/unit/UnitLength.h
        src/unit/UnitManager.h
        src/util/Config.h
        src/util/Global.h
#        src/util/Data.h
        src/util/Number.h
        src/util/String.h
        src/util/Singleton.h
#        src/util/DataManipulation.h
        src/util/Logger.h
        )

set (atsdb_MOC_HEADERS
        src/filter/DBFilterCondition.h
        src/filter/DBFilterWidget.h
        src/filter/FilterEditWidget.h
        src/filter/FilterGeneratorWidget.h
        src/filter/SensorFilterWidget.h
        src/gui/DBSelectionWidget.h
        src/gui/QCollapsableWidget.h
        src/gui/WorkerThreadWidget.h
#        src/gui/WorkflowEditWidget.h
#        src/gui/WorkflowEditDialog.h
#        src/gui/WorkflowSettingsEditDialog.h
#        src/gui/ComputationEditDialog.h
#        src/gui/TransformationEditDialog.h
#        src/gui/TransformationVariableEditDialog.h
        src/gui/FilterEditDialog.h
        src/object/DBOTypeComboBox.h
        src/object/DBObjectWidget.h
        src/object/DBObjectEditWidget.h
        src/object/DBOVariableComboBox.h
        src/object/DBOVariableSetWidget.h
        src/object/DBOVariableOrderedSetWidget.h
        src/object/DBOVariableSelectionWidget.h
#        src/object/DBOVariableDataTypeComboBox.h
        src/object/PropertyDataTypeComboBox.h
        src/object/MetaDBObjectEditWidget.h
#        src/object/StringRepresentationComboBox.h
        src/object/UnitSelectionWidget.h
        src/polygon/AirspaceSectorManagerWidget.h
        src/polygon/AirspaceSectorTreeWidget.h
        src/polygon/AirspaceSectorWidget.h
        src/projection/ProjectionManagerWidget.h
        src/schema/DBTableColumnComboBox.h
        src/schema/DBSchemaWidget.h
        src/schema/DBSchemaEditWidget.h
        src/schema/DBTableEditWidget.h
        src/schema/MetaDBTableEditWidget.h
        )

set ( atsdb_SRCS
#        src/ATSDB.cpp
         src/buffer/ArrayList.cpp
         src/buffer/Buffer.cpp
        src/buffer/BufferFilter.cpp
        src/config/Configurable.cpp
        src/config/Configuration.cpp
        src/config/ConfigurationManager.cpp
        src/command/DBCommand.cpp
        src/command/DBCommandList.cpp
        src/connection/BufferWriter.cpp
        src/connection/DBInterface.cpp
#        src/connection/MySQLConConnection.cpp
        src/connection/MySQLppConnection.cpp
#        src/connection/SQLiteConnection.cpp
        src/datasource/DataSource.cpp
        src/filter/DBFilter.cpp
        src/filter/DBFilterCondition.cpp
        src/filter/DBFilterWidget.cpp
        src/filter/FilterManager.cpp
        src/filter/SensorFilter.cpp
        src/filter/SensorFilterWidget.cpp
        src/filter/FilterEditWidget.cpp
        src/filter/FilterGeneratorWidget.cpp
        src/gui/DBSelectionWidget.cpp
        src/gui/QCollapsableWidget.cpp
        src/gui/WorkerThreadWidget.cpp
#        src/gui/WorkflowEditWidget.cpp
#        src/gui/WorkflowEditDialog.cpp
#        src/gui/WorkflowSettingsEditDialog.cpp
#        src/gui/ComputationEditDialog.cpp
#        src/gui/TransformationEditDialog.cpp
#        src/gui/TransformationVariableEditDialog.cpp
        src/gui/FilterEditDialog.cpp
#        src/job/DBOActiveDataSourcesDBJob.cpp
#        src/job/DBOVariableDistinctStatisticsDBJob.cpp
#        src/job/DBOVariableMinMaxDBJob.cpp
#        src/job/DBOCountDBJob.cpp
#        src/job/DBOReadDBJob.cpp
#        src/job/DBOInfoDBJob.cpp
#        src/job/DBJob.cpp
#        src/job/Job.cpp
#        src/job/JobOrderer.cpp
#        src/job/FinalizeDBOReadJob.cpp
#        src/job/UpdateBufferDBJob.cpp
#        src/job/WriteBufferDBJob.cpp
#        src/job/PostProcessDBJob.cpp
#        src/job/TransformationJob.cpp
#        src/mem/MemoryManager.cpp
        src/schema/DBSchemaWidget.cpp
        src/schema/DBSchemaEditWidget.cpp
        src/schema/DBTableEditWidget.cpp
        src/schema/DBTableColumnComboBox.cpp
        src/schema/MetaDBTableEditWidget.cpp
        src/object/DBOVariableComboBox.cpp
        src/object/DBOVariableSetWidget.cpp
        src/object/DBOVariableOrderedSetWidget.cpp
        src/object/DBOVariableSelectionWidget.cpp
        src/object/DBObjectEditWidget.cpp
        src/object/DBObjectWidget.cpp
        src/object/MetaDBObjectEditWidget.cpp
        src/object/UnitSelectionWidget.cpp
        src/object/DBObject.cpp
        src/object/DBObjectManager.cpp
        src/object/DBOVariable.cpp
        src/object/DBOVariableSet.cpp
        src/object/DBOVariableOrderedSet.cpp
        src/polygon/Polygon4DDI.cpp
        src/polygon/GeographicPoint.cpp
        src/polygon/AirspaceSector.cpp
        src/polygon/AirspaceSectorManager.cpp
        src/polygon/AirspaceSectorManagerWidget.cpp
        src/polygon/AirspaceSectorTreeWidget.cpp
        src/polygon/AirspaceSectorWidget.cpp
#        src/polygon/acgxml/ACGXMLParser.cpp
#        src/polygon/acgxml/Abd.cpp
#        src/polygon/acgxml/Ase.cpp
        src/polygon/shapefile/ShapeFileParser.cpp
        src/projection/ProjectionManager.cpp
        src/projection/ProjectionManagerWidget.cpp
        src/property/Property.cpp
#        src/property/PropertyContainer.cpp
#        src/result/BufferSet.cpp
#        src/result/DBResult.cpp
        src/schema/DBTable.cpp
        src/schema/DBTableColumn.cpp
        src/schema/DBSchema.cpp
        src/schema/DBSchemaManager.cpp
        src/schema/MetaDBTable.cpp
        src/schema/SQLGenerator.cpp
#        src/struct/StructureDescription.cpp
#        src/struct/StructureReader.cpp
#        src/struct/StructureDescriptionManager.cpp
#        src/struct/StructureVariable.cpp
        src/thread/Thread.cpp
        src/thread/TimedThread.cpp
        src/thread/WorkerThread.cpp
        src/thread/WorkerThreadManager.cpp
#        src/computation/Workflow.cpp
#        src/computation/Computation.cpp
#        src/computation/ComputationElement.cpp
#        src/computation/ComputationPipeline.cpp
#        src/computation/ComputationPipelineQueue.cpp
#        src/computation/TransformationBase.cpp
#        src/computation/Transformation.cpp
#        src/computation/TransformationVariable.cpp
#        src/computation/TransformationFactory.cpp
#        src/transform/CalculateHistogram.cpp
#        src/transform/CalculateHistogram2D.cpp
#        src/transform/CalculatePosition.cpp
#        src/transform/CalculatePosition2D.cpp
#        src/transform/CalculateLines.cpp
#        src/transform/CalculateStyle.cpp
        src/unit/Unit.cpp
        src/unit/UnitManager.cpp
        src/util/Config.cpp
        src/util/Logger.cpp
#        src/util/Data.cpp
        src/util/Number.cpp
        src/util/String.cpp
        )

set ( atsdb_UI
        src/gui/FilterEditDialogBase.ui
        src/gui/ComputationEditDialogBase.ui
        src/gui/TransformationEditDialogBase.ui
        src/gui/TransformationVariableEditDialogBase.ui
        src/gui/WorkflowSettingsEditDialogBase.ui
        )

set ( client_SRCS
        src/client/client_main.cpp
    )

	
QT4_WRAP_UI( UI_HDRS ${atsdb_UI} )
qt4_wrap_cpp (atsdb_MOC_OUTFILES ${atsdb_MOC_HEADERS})
add_library ( atsdb SHARED ${atsdb_SRCS} ${atsdb_HDRS} ${UIS} ${UI_HDRS} ${RSCS} ${TRS} ${atsdb_MOC_OUTFILES} )
target_link_libraries ( atsdb  ${GDAL_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${TINYXML2_LIBRARY} ${GDAL_LIBRARIES} ${Boost_LIBRARIES} -lmysqlcppconn ${LOG4CPP_LIBRARIES} ${MySQLpp_LIBRARY})
# ${SQLITE3_LIBRARIES} ${Boost_SYSTEM_LIBRARY} ${Boost_DATA_TIME_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_IOSTREAMS_LIBRARY}

add_executable ( atsdb_client ${atsdb_HDRS} ${client_SRCS})
target_link_libraries ( atsdb_client atsdb ${GDAL_LIBRARIES} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${TINYXML2_LIBRARY} ${GDAL_LIBRARIES} ${Boost_LIBRARIES} -lmysqlcppconn ${LOG4CPP_LIBRARIES} ${MySQLpp_LIBRARY})

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




        
