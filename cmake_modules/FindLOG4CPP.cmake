#
# Locate log4cpp include paths and libraries
# log4cpp can be found at http://log4cpp.sourceforge.net/
# Written by Manfred Ulz, manfred.ulz_at_tugraz.at

# This module defines
# LOG4CPP_INCLUDE_DIR, where to find ptlib.h, etc.
# LOG4CPP_LIBRARIES, the libraries to link against to use pwlib.
# LOG4CPP_FOUND, If false, don't try to use pwlib.

FIND_PATH(LOG4CPP_INCLUDE_DIR log4cpp/Category.hh
PATHS
"$ENV{LOG4CPP}/include"
/usr/local/include
/usr/include
)

FIND_LIBRARY(LOG4CPP_LIBRARIES log4cpp
PATHS
"$ENV{LOG4CPP}/lib"
/usr/local/lib
/usr/lib
)

SET(LOG4CPP_FOUND 0)
IF(LOG4CPP_INCLUDE_DIR)
IF(LOG4CPP_LIBRARIES)
SET(LOG4CPP_FOUND 1)
MESSAGE(STATUS "Found Log4CPP")
ENDIF(LOG4CPP_LIBRARIES)
ENDIF(LOG4CPP_INCLUDE_DIR)

MARK_AS_ADVANCED(
LOG4CPP_INCLUDE_DIR
LOG4CPP_LIBRARIES
) 
