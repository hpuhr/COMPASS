#
# Locate jASTERIX include paths and libraries

# This module defines
# jASTERIX_INCLUDE_DIR, where to find jASTERIX.h, etc.
# jASTERIX_LIBRARIES, the libraries to link against to use pwlib.
# jASTERIX_FOUND, If false, don't try to use pwlib.

#FIND_PATH(jASTERIX_INCLUDE_DIR jasterix/jasterix.h
#PATHS "$ENV{jASTERIX}/include" /usr/local/include /usr/include
#PATH_SUFFIXES include
#)

  find_path(jASTERIX_INCLUDE_DIR jasterix/jasterix.h
#      HINTS ${TBB_SEARCH_DIR}
      PATHS /usr/local/include /usr/include
#      PATH_SUFFIXES jasterix
)

FIND_LIBRARY(jASTERIX_LIBRARIES jasterix
PATHS
"$ENV{jASTERIX}/lib"
/usr/local/lib
/usr/lib
)

SET(jASTERIX_FOUND 0)
IF(jASTERIX_INCLUDE_DIR)
IF(jASTERIX_LIBRARIES)
SET(jASTERIX_FOUND 1)
MESSAGE(STATUS "Found jASTERIX")
ENDIF(jASTERIX_LIBRARIES)
ENDIF(jASTERIX_INCLUDE_DIR)

MARK_AS_ADVANCED(
jASTERIX_INCLUDE_DIR
jASTERIX_LIBRARIES
) 

