include( ../common.pro )

DEFINES += MAKE_KEXIDB_SQLITE_DRIVER_LIB

system( bash kmoc )

SOURCES = \
sqliteconnection.cpp \
sqlitedriver.cpp \
sqlitecursor.cpp \
../sqlite/sqlitekeywords.cpp \
sqlitepreparedstatement.cpp

HEADERS		= 

