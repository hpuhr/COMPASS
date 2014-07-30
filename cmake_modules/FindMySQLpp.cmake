# Find mysqlclient C++
# Find the native MySQL for C++ includes and library
#
#  MySQLpp_INCLUDE_DIR - where to find mysql.h, etc.
#  MySQLpp_LIBRARIES   - List of libraries when using MySQL.
#  MySQLpp_FOUND       - True if MySQL found.
	
if (MySQLpp_INCLUDE_DIR)
  # Already in cache, be silent
  set (MySQLpp_FIND_QUIETLY TRUE)
endif (MySQLpp_INCLUDE_DIR)
	
find_path(MySQLpp_INCLUDE_DIR mysql++.h
  /opt/local/include/mysql++
  /usr/local/include/mysql++
  /usr/include/mysql++
)
	
set(MySQLpp_NAMES mysqlpp)
find_library(MySQLpp_LIBRARY
  NAMES ${MySQLpp_NAMES}
  PATHS /opt/local/lib /usr/local/lib /usr/lib
  PATH_SUFFIXES mysql
)
	
if (MySQLpp_INCLUDE_DIR AND MySQLpp_LIBRARY)
  set(MySQLpp_FOUND TRUE)
  set( MySQLpp_LIBRARIES ${MySQLpp_LIBRARY} )
else (MySQLpp_INCLUDE_DIR AND MySQLpp_LIBRARY)
  set(MySQLpp_FOUND FALSE)
  set( MySQLpp_LIBRARIES )
endif (MySQLpp_INCLUDE_DIR AND MySQLpp_LIBRARY)

if (MySQLpp_FOUND)
  if (NOT MySQLpp_FIND_QUIETLY)
    message(STATUS "Found MySQL: ${MySQLpp_LIBRARY}")
  endif (NOT MySQLpp_FIND_QUIETLY)
else (MySQLpp_FOUND)
  if (MySQLpp_FIND_REQUIRED)
   message(STATUS "Looked for MySQL libraries named ${MySQLpp_NAMES}.")
   message(FATAL_ERROR "Could NOT find MySQL library")
  endif (MySQLpp_FIND_REQUIRED)
endif (MySQLpp_FOUND)
	
mark_as_advanced(
  MySQLpp_LIBRARY
  MySQLpp_INCLUDE_DIR
  )
