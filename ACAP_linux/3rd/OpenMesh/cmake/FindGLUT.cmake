# - try to find glut library and include files
#  GLUT_INCLUDE_DIR, where to find GL/glut.h, etc.
#  GLUT_LIBRARIES, the libraries to link against
#  GLUT_FOUND, If false, do not try to use GLUT.
# Also defined, but not for general use are:
#  GLUT_glut_LIBRARY = the full path to the glut library.
#  GLUT_Xmu_LIBRARY  = the full path to the Xmu library.
#  GLUT_Xi_LIBRARY   = the full path to the Xi Library.

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

IF (WIN32)

  # Check if the base path is set
  if ( NOT CMAKE_WINDOWS_LIBS_DIR )
    # This is the base directory for windows library search used in the finders we shipp.
    set(CMAKE_WINDOWS_LIBS_DIR "c:\libs" CACHE STRING "Default Library search dir on windows." )
  endif()

  if ( CMAKE_GENERATOR MATCHES "^Visual Studio 11.*Win64" )
    SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2012/x64/")
  elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 11.*" )
    SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2012/x32/")
  elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 12.*Win64" )
    SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2013/x64/")
  elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 12.*" )
    SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2013/x32/")
  elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 14.*Win64" )
    SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2015/x64/")
  elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 14.*" )
    SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2015/x32/")
  endif()


  FIND_PATH( GLUT_INCLUDE_DIR NAMES GL/glut.h 
    PATHS  ${GLUT_ROOT_PATH}/include 
           "${CMAKE_WINDOWS_LIBS_DIR}/glut-3.7/include"
           "${VS_SEARCH_PATH}/freeglut-3.0.0/include" 
           "${VS_SEARCH_PATH}/freeglut-2.8.1/include" )

  FIND_LIBRARY( GLUT_glut_LIBRARY NAMES glut32 glut freeglut
    PATHS
    ${OPENGL_LIBRARY_DIR}
    ${GLUT_ROOT_PATH}/Release
    "${CMAKE_WINDOWS_LIBS_DIR}/glut-3.7/lib"
    "${VS_SEARCH_PATH}/freeglut-3.0.0/lib"
    "${VS_SEARCH_PATH}/freeglut-2.8.1/lib"
    )

  GET_FILENAME_COMPONENT( GLUT_LIBRARY_DIR ${GLUT_glut_LIBRARY} PATH ) 
 
ELSE (WIN32)
  
  IF (APPLE)
    # These values for Apple could probably do with improvement.
    FIND_PATH( GLUT_INCLUDE_DIR glut.h
      /System/Library/Frameworks/GLUT.framework/Versions/A/Headers
      ${OPENGL_LIBRARY_DIR}
      )
    SET(GLUT_glut_LIBRARY "-framework GLUT" CACHE STRING "GLUT library for OSX") 
    SET(GLUT_cocoa_LIBRARY "-framework Cocoa" CACHE STRING "Cocoa framework for OSX")
  ELSE (APPLE)
    
    FIND_PATH( GLUT_INCLUDE_DIR GL/glut.h
      /usr/include/GL
      /usr/openwin/share/include
      /usr/openwin/include
      /opt/graphics/OpenGL/include
      /opt/graphics/OpenGL/contrib/libglut
      )
  
    FIND_LIBRARY( GLUT_glut_LIBRARY glut
      /usr/openwin/lib
      )
    
    FIND_LIBRARY( GLUT_Xi_LIBRARY Xi
      /usr/openwin/lib
      )
    
    FIND_LIBRARY( GLUT_Xmu_LIBRARY Xmu
      /usr/openwin/lib
      )
    
  ENDIF (APPLE)
  
ENDIF (WIN32)

SET( GLUT_FOUND "NO" )
IF(GLUT_INCLUDE_DIR)
  IF(GLUT_glut_LIBRARY)
    
    SET( GLUT_LIBRARIES
      ${GLUT_glut_LIBRARY}
      ${GLUT_cocoa_LIBRARY}
      )
    SET( GLUT_FOUND "YES" )
    
    #The following deprecated settings are for backwards compatibility with CMake1.4
    SET (GLUT_LIBRARY ${GLUT_LIBRARIES})
    SET (GLUT_INCLUDE_PATH ${GLUT_INCLUDE_DIR})
    
  ENDIF(GLUT_glut_LIBRARY)
ENDIF(GLUT_INCLUDE_DIR)

MARK_AS_ADVANCED(
  GLUT_INCLUDE_DIR
  GLUT_glut_LIBRARY
  GLUT_Xmu_LIBRARY
  GLUT_Xi_LIBRARY
  )
