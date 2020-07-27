# - Try to find IPOPT
# Once done this will define
#  IPOPT_FOUND - System has IpOpt
#  IPOPT_INCLUDE_DIRS - The IpOpt include directories
#  IPOPT_LIBRARY_DIRS - The library directories needed to use IpOpt
#  IPOPT_LIBRARIES    - The libraries needed to use IpOpt


if (IPOPT_INCLUDE_DIR)
  # in cache already
  SET(IPOPT_FIND_QUIETLY TRUE)
endif (IPOPT_INCLUDE_DIR)

if (WIN32)

  if ( CMAKE_GENERATOR MATCHES ".*Win64" )
    SET( DIRSUFFIX "lib64" )
  else ()
    SET( DIRSUFFIX "lib" )
  endif()

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
  
   find_path(IPOPT_INCLUDE_DIR NAMES IpNLP.hpp
     PATHS
	 "${VS_SEARCH_PATH}Ipopt-3.12.9/include/coin"
	 "${VS_SEARCH_PATH}Ipopt-3.12.4/Ipopt/MSVisualStudio/v8-ifort/installed/include/coin"
	 "${VS_SEARCH_PATH}Ipopt-3.11.9/Ipopt/MSVisualStudio/v8-ifort/installed/include/coin"
     "C:\\libs\\Ipopt-3.8.2\\include\\coin"
     ${IPOPT_DIR}/include
   )

   IF(IPOPT_INCLUDE_DIR)
      find_library( IPOPT_LIBRARY_RELEASE 
                    Ipopt ipopt libipopt IpOpt-vc10
                    PATHS "C:\\libs\\Ipopt-3.8.2\\lib\\win32\\release" 
					"${VS_SEARCH_PATH}Ipopt-3.12.9/lib"
                    "${VS_SEARCH_PATH}Ipopt-3.12.4/Ipopt/MSVisualStudio/v8-ifort/installed/lib"
			        "${VS_SEARCH_PATH}Ipopt-3.11.9/Ipopt/MSVisualStudio/v8-ifort/installed/lib"
				   )
      find_library( IPOPT_LIBRARY_DEBUG
                    Ipopt ipoptd libipoptd IpOpt-vc10d
                    PATHS "C:\\libs\\Ipopt-3.8.2\\lib\\win32\\debug" 
					"${VS_SEARCH_PATH}Ipopt-3.12.9/lib"
				    "${VS_SEARCH_PATH}Ipopt-3.12.4/Ipopt/MSVisualStudio/v8-ifort/installed/lib"
			        "${VS_SEARCH_PATH}Ipopt-3.11.9/Ipopt/MSVisualStudio/v8-ifort/installed/lib"
		   )

      set ( IPOPT_LIBRARY "optimized;${IPOPT_LIBRARY_RELEASE};debug;${IPOPT_LIBRARY_DEBUG}" CACHE  STRING "IPOPT Libraries" )
	  
	  GET_FILENAME_COMPONENT(IPOPT_LIBRARY_DIR ${IPOPT_LIBRARY_RELEASE} PATH )
	  MARK_AS_ADVANCED(IPOPT_LIBRARY_DIR)

      SET(IPOPT_FOUND TRUE)
      SET(IPOPT_INCLUDE_DIR ${IPOPT_INCLUDE_DIR})
	  # Todo, set right version depending on build type (debug/release)
	  #GET_FILENAME_COMPONENT( IPOPT_LIBRARY_DIR ${GLEW_LIBRARY} PATH )
    ELSE(IPOPT_INCLUDE_DIR)
      SET(IPOPT_FOUND FALSE)
      SET(IPOPT_INCLUDE_DIR ${IPOPT_INCLUDE_DIR})
    ENDIF(IPOPT_INCLUDE_DIR)

ELSE( WIN32 )
   find_path(IPOPT_INCLUDE_DIR NAMES IpNLP.hpp
     PATHS  "$ENV{IPOPT_HOME}/include/coin"
            "/usr/include/coin"
    
   )

   find_library( IPOPT_LIBRARY 
                 ipopt
                 PATHS "$ENV{IPOPT_HOME}/lib"
                       "/usr/lib" )   
    
    #wrong config under Debian workaround
    add_definitions( -DHAVE_CSTDDEF )

   
   # set optional path to HSL Solver for dynamic usage
   find_path(IPOPT_HSL_LIBRARY_DIR 
             NAMES libhsl.so
                   libhsl.dylib
             PATHS "$ENV{IPOPT_HSL_LIBRARY_PATH}"
                   "$ENV{HOME}/opt/HSL/lib"
   )

   # find HSL library for fixed linking of solvers   
   find_library( IPOPT_HSL_LIBRARY 
                 coinhsl
                 PATHS "$ENV{IPOPT_HOME}/lib"
                       "/usr/lib" )   
   
   
   IF( IPOPT_HSL_LIBRARY_DIR)
     IF( NOT IPOPT_FIND_QUIETLY )
        message ( "IPOPT_HSL_LIBRARY_DIR found at ${IPOPT_HSL_LIBRARY_DIR} ")
     ENDIF()
     set(IPOPT_LIBRARY_DIR ${IPOPT_HSL_LIBRARY_DIR})
     LIST( APPEND IPOPT_LIBRARY_DIRS "${IPOPT_HSL_LIBRARY_DIR}")
   ENDIF(IPOPT_HSL_LIBRARY_DIR)
   
   
   set(IPOPT_INCLUDE_DIRS "${IPOPT_INCLUDE_DIR}" )
   set(IPOPT_LIBRARIES "${IPOPT_LIBRARY}" )
   
   IF(IPOPT_HSL_LIBRARY)
     LIST( APPEND IPOPT_LIBRARIES "${IPOPT_HSL_LIBRARY}")   
   ENDIF(IPOPT_HSL_LIBRARY)

   include(FindPackageHandleStandardArgs)
   # handle the QUIETLY and REQUIRED arguments and set LIBIPOPT_FOUND to TRUE
   # if all listed variables are TRUE
   find_package_handle_standard_args(IPOPT  DEFAULT_MSG
                                     IPOPT_LIBRARY IPOPT_INCLUDE_DIR)

   mark_as_advanced(IPOPT_INCLUDE_DIR IPOPT_LIBRARY )
   
ENDIF()
