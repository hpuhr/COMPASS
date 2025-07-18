
FIND_PATH(DUCKDB_INCLUDE_DIR duckdb.h
PATHS
/usr/local/include
/usr/include
)

FIND_LIBRARY(DUCKDB_LIBRARIES libduckdb.so
PATHS
/usr/local/lib
/usr/lib
)

SET(DUCKDB_FOUND 0)
IF(DUCKDB_INCLUDE_DIR)
IF(DUCKDB_LIBRARIES)
SET(DUCKDB_FOUND 1)
MESSAGE(STATUS "Found DuckDB")
ENDIF(DUCKDB_LIBRARIES)
ENDIF(DUCKDB_INCLUDE_DIR)

MARK_AS_ADVANCED(
DUCKDB_INCLUDE_DIR
DUCKDB_LIBRARIES
) 
