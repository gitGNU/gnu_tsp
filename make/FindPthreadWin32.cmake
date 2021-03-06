
SET(PATH_DIR ${CMAKE_SOURCE_DIR}/external/pthreadwin32 c:/pthreadwin32)

MESSAGE(STATUS "Looking for PthreadWin32...")

FIND_FILE(PTHREADWIN32_INCLUDE
	NAMES pthread.h sched.h semaphore.h
	PATHS ${PATH_DIR}
	PATH_SUFFIXES include pthreads
	DOC "The PthreadWin32 Include Files")
	
FIND_LIBRARY(PTHREADWIN32_LIBRARY
	NAMES pthreadVC pthreadVC1
	PATHS ${PATH_DIR}
	PATH_SUFFIXES lib pthreads
	DOC "The PthreadWin32 Library")
	
FIND_PROGRAM(PTHREADWIN32_INSTALL
	NAMES pthreads-2005-03-08.exe
	PATHS ${PATH_DIR}
	PATH_SUFFIXES source
	DOC "The PthreadWin32 install program")

###########################################
# TO DO : Find the DLL : pthreadVC*.dll
###########################################

IF (PTHREADWIN32_INCLUDE AND PTHREADWIN32_LIBRARY)
  MESSAGE(STATUS "Looking for PthreadWin32... - found PthreadWin32 lib is ${PTHREADWIN32_LIBRARY} and include is ${PTHREADWIN32_INCLUDE}")
  SET(PTHREADWIN32_FOUND "YES")
  GET_FILENAME_COMPONENT(PTHREADWIN32_LIBRARY_DIRS ${PTHREADWIN32_LIBRARY} PATH)
  GET_FILENAME_COMPONENT(PTHREADWIN32_INCLUDE_DIRS ${PTHREADWIN32_INCLUDE} PATH)
ELSE (PTHREADWIN32_INCLUDE AND PTHREADWIN32_LIBRARY)
  SET(PTHREADWIN32_FOUND "NO")
  MESSAGE(STATUS "Looking for PthreadWin32... - NOT found")
ENDIF (PTHREADWIN32_INCLUDE AND PTHREADWIN32_LIBRARY)
