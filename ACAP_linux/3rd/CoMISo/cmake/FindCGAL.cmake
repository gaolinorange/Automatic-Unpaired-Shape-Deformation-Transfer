# - Find CGAL
# Find the CGAL autolinking headers.
#
#  CGAL_INCLUDE_DIR -  where to find CGAL.h, etc.
#  CGAL_FOUND        - True if CGAL found.

IF (CGAL_INCLUDE_DIR)
  # Already in cache, be silent
  SET(CGAL_FIND_QUIETLY TRUE)
ENDIF (CGAL_INCLUDE_DIR)

GET_FILENAME_COMPONENT(module_file_path ${CMAKE_CURRENT_LIST_FILE} PATH )

# Check if the base path is set
if ( NOT CMAKE_WINDOWS_LIBS_DIR )
  # This is the base directory for windows library search used in the finders we shipp.
  set(CMAKE_WINDOWS_LIBS_DIR "c:/libs" CACHE STRING "Default Library search dir on windows." )
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
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 15.*Win64" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2017/x64/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 15.*" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2017/x32/")
endif()

IF (NOT APPLE )
	# Look for the header file.
	FIND_PATH(CGAL_INCLUDE_DIR NAMES CGAL/auto_link/auto_link.h
        	                   PATHS "${VS_SEARCH_PATH}/CGAL-4.12/include"
                                     "${VS_SEARCH_PATH}/CGAL-4.7/include"
                                      /usr/include
                                     ../../External/include
                                     $ENV{CGAL_DIR}/include									 
                                     "C:/libs/CGAL-3.6/include"
                                     "C:/Program Files/CGAL-3.5/include"
                                     "C:/Programme/CGAL-3.5/include"
                                     "C:/libs/CGAL-3.5/include"
                                     "C:/Program Files/CGAL-3.4/include"
                                     "C:/Programme/CGAL-3.4/include"
                                     "C:/libs/CGAL-3.4/include"
                                     "C:/Programme/CGAL-3.9/include"
                                     "C:/Program Files/CGAL-3.9/include"
                                     "C:/Program Files (x86)/CGAL-3.9/include"
                                     ${module_file_path}/../../../External/include )
ELSE( NOT APPLE)
	# Look for the header file.
	FIND_PATH(CGAL_INCLUDE_DIR NAMES CGAL/auto_link/auto_link.h
        	                   PATHS "/opt/local/include"
                 )

ENDIF( NOT APPLE )



MARK_AS_ADVANCED(CGAL_INCLUDE_DIR)

if ("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}" MATCHES ".*clang.*")
  SET(CXX_IS_CLANG true)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "[cC][lL][aA][nN][gG]")
  SET(CXX_IS_CLANG true)
else()
  SET(CXX_IS_CLANG false)
endif()

# Copy the results to the output variables.
IF(CGAL_INCLUDE_DIR )
  SET(CGAL_FOUND 1)
  SET(CGAL_INCLUDE_DIR ${CGAL_INCLUDE_DIR})
  # Look for the taucs dir.
  FIND_PATH(CGAL_TAUCS_DIR NAMES include/taucs.h
                           PATHS ${CGAL_INCLUDE_DIR}/../auxiliary/taucs)

  IF(WIN32)
    include(CGAL_GeneratorSpecificSettings)
    find_path(CGAL_LIBRARY_DIR 
                NAMES "CGAL-${CGAL_TOOLSET}-mt.lib" 
                      "CGAL-${CGAL_TOOLSET}-mt-gd.lib" 
                      "CGAL-${CGAL_TOOLSET}-mt-4.12.lib" 
                      "CGAL-${CGAL_TOOLSET}-mt-4.7.lib" 
                      "CGAL-${CGAL_TOOLSET}-mt-4.5.lib"
                PATHS "${CGAL_INCLUDE_DIR}/../lib"
                DOC "Directory containing the CGAL library"
               ) 
			
    # Binary dir for DLLs			
	find_path(CGAL_BIN_DIR 
                NAMES "CGAL-${CGAL_TOOLSET}-mt-4.12.dll" 
                      "CGAL-${CGAL_TOOLSET}-mt-4.7.dll" 
                      "CGAL-${CGAL_TOOLSET}-mt-4.5.dll" 
                      "CGAL-${CGAL_TOOLSET}-mt-gd.dll"
                PATHS "${CGAL_INCLUDE_DIR}/../bin"
                DOC "Directory containing the CGAL DLLs"
               ) 
			   
	MARK_AS_ADVANCED(CGAL_BIN_DIR )
    #add_definitions (-DCGAL_AUTOLINK)
  ELSEIF( APPLE)
    find_path(CGAL_LIBRARY_DIR 
                NAMES "libCGAL.dylib"
                PATHS "/opt/local/lib/"
                DOC "Directory containing the CGAL library"
               ) 
    list ( APPEND CGAL_LIBRARIES CGAL CGAL_Core CGAL_ImageIO mpfr )
    if (NOT CXX_IS_CLANG)
        set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -frounding-math")
        set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -frounding-math")
    endif()

  ELSE( WIN32 )
    find_path(CGAL_LIBRARY_DIR
              NAMES "libCGAL.so"
              PATHS "/usr/lib/" "/usr/lib64" 
              DOC "Directory containing the CGAL library"
             )
    list ( APPEND CGAL_LIBRARIES CGAL CGAL_Core CGAL_ImageIO) 
    if (NOT CXX_IS_CLANG)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frounding-math")
    endif()


    # This is needed to link correctly against lapack
    add_definitions (-DCGAL_USE_F2C)
  ENDIF(WIN32)


 

ELSE(CGAL_INCLUDE_DIR )
  SET(CGAL_FOUND 0)
  SET(CGAL_INCLUDE_DIR)
ENDIF(CGAL_INCLUDE_DIR )

#use eigen if available
find_package(EIGEN3)
if ( EIGEN3_FOUND )
  add_definitions(-DCGAL_EIGEN3_ENABLED)
endif()

# Report the results.
IF(NOT CGAL_FOUND)
  SET(CGAL_DIR_MESSAGE
    "CGAL was not found. Make sure CGAL_INCLUDE_DIR is set to the directories containing the include files for CGAL. .")
  IF(CGAL_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "${CGAL_DIR_MESSAGE}")
  ELSEIF(NOT CGAL_FIND_QUIETLY)
    MESSAGE(STATUS "${CGAL_DIR_MESSAGE}")
  ELSE(NOT CGAL_FIND_QUIETLY)
  ENDIF(CGAL_FIND_REQUIRED)
ELSE (NOT CGAL_FOUND)
  IF(NOT CGAL_FIND_QUIETLY)
    MESSAGE(STATUS "Looking for CGAL - found")
  ENDIF(NOT CGAL_FIND_QUIETLY)
ENDIF(NOT CGAL_FOUND)

