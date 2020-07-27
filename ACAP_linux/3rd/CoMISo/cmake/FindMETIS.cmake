# - Try to find METIS
# Once done this will define
#  METIS_FOUND - System has Metis
#  METIS_INCLUDE_DIRS - The Metis include directories
#  METIS_LIBRARY_DIRS - The library directories needed to use Metis
#  METIS_LIBRARIES    - The libraries needed to use Metis

if (METIS_INCLUDE_DIR)
  # in cache already
  SET(METIS_FIND_QUIETLY TRUE)
endif (METIS_INCLUDE_DIR)

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


find_path(METIS_INCLUDE_DIR NAMES metis.h
     PATHS "$ENV{IPOPT_HOME}/ThirdParty/Metis/metis-4.0/Lib/"
           "/usr/include/"
           "/usr/include/metis"
           "/opt/local/include"
           "/opt/local/include/metis"
		   "${VS_SEARCH_PATH}Ipopt-3.12.9/include/metis"
           "${VS_SEARCH_PATH}Ipopt-3.12.4/Ipopt/MSVisualStudio/v8-ifort/installed/include/metis"
   )
   
find_library( METIS_LIBRARY_RELEASE
              metis coinmetis
              PATHS "$ENV{IPOPT_HOME}/lib/"
                    "/usr/lib"
                    "/opt/local/lib"
					"${VS_SEARCH_PATH}Ipopt-3.12.9/lib"
                    "${VS_SEARCH_PATH}Ipopt-3.12.4/Ipopt/MSVisualStudio/v8-ifort/installed/lib"
                    )
                    
find_library( METIS_LIBRARY_DEBUG
              metisd coinmetisd
              PATHS "$ENV{IPOPT_HOME}/lib/"
                    "/usr/lib"
                    "/opt/local/lib" 
					"${VS_SEARCH_PATH}Ipopt-3.12.9/lib"
                    "${VS_SEARCH_PATH}Ipopt-3.12.4/Ipopt/MSVisualStudio/v8-ifort/installed/lib"
                    )      
                    
include(SelectLibraryConfigurations)
select_library_configurations( METIS )                   

set(METIS_INCLUDE_DIRS "${METIS_INCLUDE_DIR}" )
set(METIS_LIBRARIES "${METIS_LIBRARY}" )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBCPLEX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(METIS  DEFAULT_MSG
                                  METIS_LIBRARY METIS_INCLUDE_DIR)

mark_as_advanced(METIS_INCLUDE_DIR METIS_LIBRARY )
