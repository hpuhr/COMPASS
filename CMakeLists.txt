set(COMPASS_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
message("  Path: ${COMPASS_PATH}")

cmake_minimum_required(VERSION 3.3)

set ( CMAKE_BUILD_TYPE RelWithDebInfo ) #Debug RelWithDebInfo Release

project( COMPASS )
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "7")
SET(CPACK_PACKAGE_VERSION_PATCH "1")

message("  System: ${CMAKE_SYSTEM}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message("  Platform: Linux")
add_definitions ( -UNDEBUG -Wall -std=c++11) # -DQT_NO_KEYWORDS

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

#-Wno-deprecated-declarations -Wno-ignored-attributes -Wno-misleading-indentation for new compiler and old libs
# -UNDEBUG for enabling asserts in release buils (until testing/error management is improved)

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5OpenGL)
find_package(Qt5Charts)

message("Qt Widgets version: ${Qt5Widgets_VERSION}")

set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

#add_definitions(${Qt5Widgets_DEFINITIONS})
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

find_package ( Boost 1.73.0 REQUIRED COMPONENTS regex system program_options filesystem iostreams thread)
message("  Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message("  Boost_LIBRARIES: ${Boost_LIBRARIES}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${COMPASS_PATH}/cmake_modules)
message("  CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}") 
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH};/usr/local/lib/cmake/;/usr/lib/x86_64-linux-gnu/)
message("  CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package (Eigen3 3.2 REQUIRED)

find_package ( Sqlite3 3.35 REQUIRED )
message("  SQLite3_INCLUDE_DIR: ${SQLITE3_INCLUDE_DIR}")
message("  SQLite3_LIBRARY_DIR: ${SQLITE3_LIBRARIES}")

find_package ( LOG4CPP REQUIRED )
message("  LOG4CPP_INCLUDE_DIR: ${LOG4CPP_INCLUDE_DIR}")
message("  LOG4CPP_LIBRARY: ${LOG4CPP_LIBRARIES}") 

find_package ( GDAL REQUIRED )
message("  GDAL_INCLUDE_DIRS: ${GDAL_INCLUDE_DIRS}")
message("  GDAL_LIBRARIES: ${GDAL_LIBRARIES}")

find_package ( LibArchive REQUIRED )
message("  LibArchive_INCLUDE_DIRS: ${LibArchive_INCLUDE_DIRS}")
message("  LibArchive_LIBRARIES: ${LibArchive_LIBRARIES}")

find_package(TBB REQUIRED)
message("  TBB_INCLUDE_DIRS: ${TBB_INCLUDE_DIRS}")
message("  TBB_LIBRARIES: ${TBB_LIBRARIES}")

find_package ( jASTERIX REQUIRED)
message("  jASTERIX found: ${jASTERIX_FOUND}")
message("  jASTERIX_INCLUDE_DIR: ${jASTERIX_INCLUDE_DIR}")
message("  jASTERIX_LIBRARIES: ${jASTERIX_LIBRARIES}")

find_package ( OpenSSL REQUIRED )
message("  OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
message("  OPENSSL_CRYPTO_LIBRARY: ${OPENSSL_CRYPTO_LIBRARY}")

find_package(NemaTode REQUIRED CONFIG)


# google profiler not working
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
#set (CMAKE_SHARED_LINKER_FLAGS "-pg")
#set(CMAKE_EXE_LINKER_FLAGS "-pg")

find_package(Threads REQUIRED)

add_library(compass "")

message("Install path: ${CMAKE_INSTALL_PREFIX}")

include(lib/CMakeLists.txt)
include(src/CMakeLists.txt)

set (EXPERIMENTAL_SRC true)
message("Experimental source: ${EXPERIMENTAL_SRC}")

IF (EXPERIMENTAL_SRC)
    IF(IS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/experimental_src")
        MESSAGE(STATUS "Experimental source found")
    ELSE()
        MESSAGE(STATUS "Experimental source not found found, deactivating")
        set (EXPERIMENTAL_SRC false)
    ENDIF()
ENDIF()

IF (EXPERIMENTAL_SRC)
    MESSAGE(STATUS "Including experimental source")
    include(experimental_src/CMakeLists.txt)
ENDIF()

configure_file(src/util/global.h.in include/global.h)

# includes and library linking

include_directories (
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/include
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    ${Qt5Charts_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIR}
    ${LOG4CPP_INCLUDE_DIR}
    ${GDAL_INCLUDE_DIRS}
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${LibArchive_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    ${JSONCPP_INCLUDE_DIR}
    ${jASTERIX_INCLUDE_DIR}
    )

link_directories(
    ${Boost_LIBRARY_DIR}
    ${jASTERIX_LIBRARIES}
    )

target_link_libraries ( compass
    Qt5::Widgets
    Qt5::Core
    Qt5::OpenGL
    Qt5::Charts
    ${jASTERIX_LIBRARIES}
    ${Boost_LIBRARIES}
    ${LOG4CPP_LIBRARIES}
    ${GDAL_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${LibArchive_LIBRARIES}
    ${JSONCPP_LIBRARY}
    ${TBB_LIBRARIES}
    TBB::tbb
    ${OPENSSL_CRYPTO_LIBRARY}
    NemaTode::NemaTode
    Threads::Threads
    )

# installation stuff

message("Installing using prefix: ${CMAKE_INSTALL_PREFIX}")
install(DIRECTORY "conf" DESTINATION compass)
install(DIRECTORY "data" DESTINATION compass)

install (TARGETS compass DESTINATION lib
    PUBLIC_HEADER DESTINATION include/compass)

install (TARGETS compass_client DESTINATION bin)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
include (CPack)


# add a target to generate API documentation with Doxygen
#find_package(Doxygen)
#if(DOXYGEN_FOUND)
#    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in @ONLY)
#    add_custom_target(doc
#        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
#        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
#        COMMENT "Generating API documentation with Doxygen" VERBATIM
#        )
#endif(DOXYGEN_FOUND)

#file(GLOB DOT ".*")
#file(GLOB TILD "*~")
#file(GLOB TEMP "*.cmake")





