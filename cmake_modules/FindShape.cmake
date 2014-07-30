# Find shape lib
#
#  Shape_INCLUDE_DIR - where to find headers
#  Shape_LIBRARIES   - List of libraries
#  Shape_FOUND       - True if found.
	
if (Shape_INCLUDE_DIR)
  # Already in cache, be silent
  set (Shape_FIND_QUIETLY TRUE)
endif (Shape_INCLUDE_DIR)
	
find_path(Shape_INCLUDE_DIR shapefil.h
  /usr/include/
  /usr/include/libshp/
)
	
set(Shape_NAMES shp)
find_library(Shape_LIBRARY
  NAMES ${Shape_NAMES}
  PATHS /opt/local/lib /usr/local/lib /usr/lib
  PATH_SUFFIXES shp
)
	
if (Shape_INCLUDE_DIR AND Shape_LIBRARY)
  set(Shape_FOUND TRUE)
  set( Shape_LIBRARIES ${Shape_LIBRARY} )
else (Shape_INCLUDE_DIR AND Shape_LIBRARY)
  set(Shape_FOUND FALSE)
  set( Shape_LIBRARIES )
endif (Shape_INCLUDE_DIR AND Shape_LIBRARY)

if (Shape_FOUND)
  if (NOT Shape_FIND_QUIETLY)
    message(STATUS "Found Shape: ${Shape_LIBRARY}")
  endif (NOT Shape_FIND_QUIETLY)
else (Shape_FOUND)
  if (Shape_FIND_REQUIRED)
   message(STATUS "Looked for Shape libraries named ${Shape_NAMES}.")
   message(FATAL_ERROR "Could NOT find Shape library")
  endif (Shape_FIND_REQUIRED)
endif (Shape_FOUND)
	
mark_as_advanced(
  Shape_LIBRARY
  Shape_INCLUDE_DIR
  )
