
## Add compilable consumer sub-directories

ADD_SUBDIRECTORY(generic)
ADD_SUBDIRECTORY(stdout)

IF (NOT WIN32)
  ADD_SUBDIRECTORY(stdout_callback)
  ADD_SUBDIRECTORY(test_group)
  ADD_SUBDIRECTORY(tutorial)
ENDIF (NOT WIN32)

IF (FUSE_FOUND)
  ADD_SUBDIRECTORY(tspfs)
ENDIF (FUSE_FOUND)

IF (NOT WIN32)
  ADD_SUBDIRECTORY(ascii_writer)
  ADD_SUBDIRECTORY(res_writer)
ENDIF (NOT WIN32)

IF (GTK_FOUND AND NOT WIN32)
  ADD_SUBDIRECTORY(gdisp)
  ADD_SUBDIRECTORY(gdisp+)
ENDIF (GTK_FOUND AND NOT WIN32)

IF (NOT WIN32)
  FIND_PACKAGE(X11)
  IF (OPENGL_FOUND AND OPENGL_GLU_FOUND AND GLUT_FOUND AND X11_FOUND)
    ADD_SUBDIRECTORY(Visu3D)
  ELSE (OPENGL_FOUND AND OPENGL_GLU_FOUND AND GLUT_FOUND AND X11_FOUND)
    MESSAGE(STATUS "Cannot build Visu3D some lib is missing")
  ENDIF (OPENGL_FOUND AND OPENGL_GLU_FOUND AND GLUT_FOUND AND X11_FOUND)
ENDIF (NOT WIN32)
