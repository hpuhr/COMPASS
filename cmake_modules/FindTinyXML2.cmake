###############################################################################
# Find TINYXML2
###############################################################################
#
# TINYXML2_FOUND
# TINYXML2_INCLUDE_DIR
# TINYXML2_LIBRARY
#
###############################################################################

find_path(TINYXML2_INCLUDE_DIR tinyxml2.h
HINTS $ENV{TINYXML2DIR}
PATH_SUFFIXES include
PATHS /usr/local
/usr
)

find_library(TINYXML2_LIBRARY
tinyxml2
HINTS $ENV{TINYXML2DIR}
PATH_SUFFIXES lib64 lib
PATHS ~/Library/Frameworks
/Library/Frameworks
/usr/local
/usr
)
