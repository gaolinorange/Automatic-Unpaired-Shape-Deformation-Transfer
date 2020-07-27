# - Try to find CBC
# Once done this will define
#  CBC_FOUND - System has CBC
#  CBC_INCLUDE_DIRS - The CBC include directories
#  CBC_LIBRARIES - The libraries needed to use CBC


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


if (NOT CBC_FOUND )

find_path(CBC_INCLUDE_DIR 
          NAMES CbcConfig.h
          PATHS "$ENV{CBC_DIR}/include/coin"
                 "/usr/include/coin"
                 "C:\\libs\\cbc\\include"
		 "${VS_SEARCH_PATH}CBC-2.9.7/Cbc/include"
		 "${VS_SEARCH_PATH}CBC-2.9.4/Cbc/include"
          )

find_library( CBC_LIBRARY_DEBUG
              NAMES Cbcd libCbcd
              PATHS "$ENV{CBC_DIR}/lib"
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\cbc\\lib"
		    "${VS_SEARCH_PATH}CBC-2.9.7/lib/${VS_SUBDIR}Debug"
                    "${VS_SEARCH_PATH}CBC-2.9.4/Cbc/lib"
              )

find_library( CBC_SOLVER_LIBRARY_DEBUG
              NAMES CbcSolverd libCbcSolverd
              PATHS "$ENV{CBC_DIR}/lib"
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\cbc\\lib"
	            "${VS_SEARCH_PATH}CBC-2.9.7/lib/${VS_SUBDIR}Debug"
                    "${VS_SEARCH_PATH}CBC-2.9.4/Cbc/lib"
              )

find_library( CBC_LIBRARY_RELEASE
              NAMES Cbc libCbc
              PATHS "$ENV{CBC_DIR}/lib"
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\cbc\\lib"
 		    "${VS_SEARCH_PATH}CBC-2.9.7/lib/${VS_SUBDIR}Release"
                    "${VS_SEARCH_PATH}CBC-2.9.4/Cbc/lib"
              )

find_library( CBC_SOLVER_LIBRARY_RELEASE
              NAMES CbcSolver libCbcSolver

              PATHS "$ENV{CBC_DIR}/lib"
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\cbc\\lib"
	  	    "${VS_SEARCH_PATH}CBC-2.9.7/lib/${VS_SUBDIR}Release"
                    "${VS_SEARCH_PATH}CBC-2.9.4/Cbc/lib"
              )


  include(SelectLibraryConfigurations)
  select_library_configurations( CBC )
  select_library_configurations( CBC_SOLVER )

set(CBC_INCLUDE_DIRS "${CBC_INCLUDE_DIR}" )
set(CBC_LIBRARIES "${CBC_LIBRARIES};${CBC_SOLVER_LIBRARIES}" )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set CBC_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(CBC  DEFAULT_MSG
                                  CBC_LIBRARY CBC_SOLVER_LIBRARY CBC_INCLUDE_DIR CBC_LIBRARIES)

mark_as_advanced(CBC_INCLUDE_DIR CBC_LIBRARY CBC_SOLVER_LIBRARY CBC_LIBRARIES)

endif(NOT CBC_FOUND)
